extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <SDL2/SDL.h>
}

#include "video_state.h"
#include "stdafx.h"
#include "window.h"
#include <GL/glew.h>

const std::string VideoState::CookiesFile = "youtube-dl-cookies.txt";

void PacketQueue::Init(){
	mutex = SDL_CreateMutex();
	cond = SDL_CreateCond();
}

int PacketQueue::Get(VideoState *vState, AVPacket *packet, bool block){

	AVPacketList *pkt1;

	SDL_LockMutex(mutex);

	int ret = -1;

	while (true){

		if (vState->quit){
			ret = -1;
			break;
		}

		pkt1 = firstPacket;

		if (pkt1){

			if (!(firstPacket = pkt1->next)) lastPacket = NULL;

			numPackets--;
			size -= pkt1->pkt.size;

			*packet = pkt1->pkt;

			av_free(pkt1);

			ret = 1;
			break;

		}
		else if (!block){

			ret = 0;
			break;

		}
		else {
			SDL_CondWait(cond, mutex);
		}
	}


	SDL_UnlockMutex(mutex);
	return ret;
}

int PacketQueue::Put(AVPacket *packet, AVPacket *flushPacket){

	if (!packet) return -1;

	AVPacketList *pkt1;

	if (packet != flushPacket && av_dup_packet(packet) < 0) return -1;

	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));

	if (!pkt1) return -1;

	pkt1->pkt = *packet;
	pkt1->next = NULL;

	SDL_LockMutex(mutex);

	if (!lastPacket)
		firstPacket = pkt1;
	else
		lastPacket->next = pkt1;

	lastPacket = pkt1;

	numPackets++;
	size += pkt1->pkt.size;

	SDL_CondSignal(cond);

	SDL_UnlockMutex(mutex);

	return 0;
}
void PacketQueue::Flush(){

	AVPacketList *pkt, *pkt1;

	SDL_LockMutex(mutex);

	for (pkt = firstPacket; pkt; pkt = pkt1) {
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}

	firstPacket = NULL;
	lastPacket = NULL;
	numPackets = 0;
	size = 0;

	SDL_UnlockMutex(mutex);
}

void PacketQueue::Destroy(){
	Flush();
	SDL_DestroyMutex(mutex);
	SDL_DestroyCond(cond);
}

double Clock::Get(){
	if (paused) return pts;

	double time = av_gettime_relative() / 1000000.0;
	return ptsDrift + time - (time - lastUpdated) * (1.0 - speed);
}

void Clock::Set(double pts){
	double time = av_gettime_relative() / 1000000.0;
	SetAt(pts, time);
}

void Clock::SetAt(double pts, double time){

	this->pts = pts;
	this->lastUpdated = time;
	this->ptsDrift = pts - time;
}

void Clock::SetToSlave(Clock *slave){
	double ourClock = Get();
	double slaveClock = slave->Get();
	if (!isnan(slaveClock) && (isnan(ourClock) || fabs(ourClock - slaveClock) > AV_NOSYNC_THRESHOLD))
		Set(slaveClock);
}

void Clock::SetSpeed(double s){

	Set(Get());
	speed = s;
}

void Clock::Init(int t){
	type = t;
	speed = 1.0;
	paused = 0;
	Set(NAN);
}

static std::string ReadUntilChar(const char *buff, int *index, char stopChar){

	std::string ret;
	int textLen = (int)strlen(buff);

	int j;
	for (j = 0; j < textLen; j++){
		char c = buff[j];
		if (c == stopChar) break;
		ret += c;
	}

	*index += j + 1;
	return ret;
}

static int StrToInt(std::string str){
	int ret = 0;
	sscanf_s(str.c_str(), "%i", &ret);
	return ret;
}

static int SubtitleThread(void *data){

	VideoState *state = (VideoState *)data;
	SubPicture *sp;
	AVPacket pkt1, *pkt = &pkt1;
	int got_subtitle;
	double pts;

	for (;;) {

		if (state->subtitleQueue.Get(state, pkt, true) < 0 || state->quit)
			break;

		SDL_LockMutex(state->subQueueMutex);

		while (state->subQueueSize >= SUBPICTURE_QUEUE_SIZE && !state->quit){
			SDL_CondWait(state->subQueueCond, state->subQueueMutex);
		}

		SDL_UnlockMutex(state->subQueueMutex);

		if (state->quit) return 0;

		sp = &state->subPictQueue[state->subPictQueueWIndex];
		/* NOTE: ipts is the PTS of the _first_ picture beginning in
		this packet, if any */
		pts = 0;
		if (pkt->pts != AV_NOPTS_VALUE)
			pts = av_q2d(state->subtitleStream->time_base) * pkt->pts;

		avcodec_decode_subtitle2(state->subtitleStream->codec, &sp->sub,
			&got_subtitle, pkt);

		if (got_subtitle) {

			sp->text = "";

			if (sp->sub.format != 0){
				for (int k = 0; k < (int)sp->sub.num_rects; k++){
					if (sp->sub.rects[k]->ass){

						struct {
							int hour;
							int minute;
							int second;
							int ms;
						} start, end;

						int marginL, marginR, marginV, marked;

						int index = 0;
						std::string format = ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ':');
						marked = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						start.hour = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ':'));
						start.minute = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ':'));
						start.second = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, '.'));
						start.ms = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						end.hour = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ':'));
						end.minute = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ':'));
						end.second = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, '.'));
						end.ms = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						std::string style = ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ',');
						std::string name = ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ',');
						marginL = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						marginR = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						marginV = StrToInt(ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ','));
						std::string effect = ReadUntilChar(&sp->sub.rects[k]->ass[index], &index, ',');
						std::string text = std::string(&sp->sub.rects[k]->ass[index]);

						// printf("%s %i %i %i %i %i %i %i %i %i %s %s %i %i %i %s %s\n",
						//  format.c_str(), marked, start.hour, start.minute, start.second, start.ms,
						//  end.hour, end.minute, end.second, end.ms, style.c_str(),name.c_str(), marginL, marginR, marginV,
						//  effect.c_str(), text.c_str());

						int textLen = (int)text.size();

						for (int k = 0; k < textLen; k++){
							if (text[k] == '{'){
								for (int f = 0; f < textLen; f++){
									if (text[f] == '}'){
										k = f + 1;
										break;
									}
								}

								if (k >= textLen){
									break;
								}
							}

							sp->text += text[k];
						}

						sp->sub.start_display_time = (int)((start.hour * 60.0f * 60.0f * 1000.0f) +
							(start.minute * 60.0f * 1000.0f) + (start.second * 1000.0f) + start.ms);

						sp->sub.end_display_time = (int)((end.hour * 60.0f * 60.0f * 1000.0f) +
							(end.minute * 60.0f * 1000.0f) + (end.second * 1000.0f) + end.ms);

						// printf("%f %f\n", (float)sp->sub.start_display_time, (float)sp->sub.end_display_time);

						// printf("%s\n", sp->sub.rects[k]->text);
					}
					else if (sp->sub.rects[k]->text){

						sp->text += sp->sub.rects[k]->text;
					}
				}
			}

			if (sp->sub.pts != AV_NOPTS_VALUE)
				pts = sp->sub.pts / (double)AV_TIME_BASE;

			sp->pts = pts;
			/* now we can update the picture count */
			if (++state->subPictQueueWIndex == SUBPICTURE_QUEUE_SIZE)
				state->subPictQueueWIndex = 0;
			SDL_LockMutex(state->subQueueMutex);
			state->subQueueSize++;
			SDL_UnlockMutex(state->subQueueMutex);
		}
		else if (got_subtitle) {
			avsubtitle_free(&sp->sub);
		}
		av_free_packet(pkt);
	}

	return 0;
}

class Cookie {
public:
	char webserver[1024];
	char readableOtherMachines[1024];
	char pathCookieRelevant[1024];
	char secureConnectionRequired[1024];
	int expireTime;
	char name[1024];
	char value[1024];
};

static std::string LoadCookies(const std::string &cookiesFile){

	std::vector<Cookie> cookies;

	FILE *fp;
	fopen_s(&fp, cookiesFile.c_str(), "rb");

	if (!fp) return "";

	while (!feof(fp)){

		if (fgetc(fp) == '#'){
			while (fgetc(fp) != '\n'){}
			continue;
		}

		fseek(fp, ftell(fp) - 1, SEEK_SET);

		Cookie cookie;
		int num = fscanf_s(fp, "%s\t", cookie.webserver);
		num += fscanf_s(fp, "%s\t", cookie.readableOtherMachines);
		num += fscanf_s(fp, "%s\t", cookie.pathCookieRelevant);
		num += fscanf_s(fp, "%s\t", cookie.secureConnectionRequired);
		num += fscanf_s(fp, "%i\t", &cookie.expireTime);
		num += fscanf_s(fp, "%s\t", cookie.name);
		num += fscanf_s(fp, "%s\n", cookie.value);

		if (num < 7) continue;

		cookies.push_back(cookie);

		// printf("%s %s %s %s %i %s %s\n",cookie.webserver, cookie.readableOtherMachines, cookie.pathCookieRelevant,
		//  cookie.secureConnectionRequired, cookie.expireTime, cookie.name, cookie.value );
	}

	// printf("\n");

	fclose(fp);

	std::string cookiesStr = "";

	for (int k = 0; k < (int)cookies.size(); k++){

		cookiesStr += std::string(cookies[k].name) + "=" + std::string(cookies[k].value) + "; ";
		cookiesStr += "path=" + std::string(cookies[k].pathCookieRelevant) + "; ";
		cookiesStr += "domain=" + std::string(cookies[k].webserver) + "; ";
		cookiesStr += "\n";
	}

	return cookiesStr;
}

static int OpenState(void *data){

	VideoState *st = (VideoState *)data;

	AVDictionary *dict = NULL;

	if (st->command.length()){


		std::string result = Window_ExecuteCommand(st->command);

		av_dict_set(&dict, "cookies", LoadCookies(VideoState::CookiesFile).c_str(), 0);

		std::string ret = "";
		for (int k = 0; k < (int)result.size(); k++)
			if ((result[k] >= 32 && result[k] <= 126))
				ret += result[k];

		if (avformat_open_input(&st->pFormatCtx, ret.c_str(), NULL, &dict) != 0)
			return -1;

	}
	else {

		if (st->cookies.size()) av_dict_set(&dict, "cookies", st->cookies.c_str(), 0);


		if (avformat_open_input(&st->pFormatCtx, st->fileName, NULL, &dict) != 0)
			return -1;
	}

	av_dict_free(&dict);

	av_dump_format(st->pFormatCtx, 0, st->fileName, 0);

	if (avformat_find_stream_info(st->pFormatCtx, NULL)<0)
		return -1;

	st->videoStreamIndex = -1;
	st->audioStreamIndex = -1;
	st->subtitleStreamIndex = -1;

	for (int i = 0; i < (int)st->pFormatCtx->nb_streams; i++){

		if (st->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && st->videoStreamIndex < 0){
			st->videoStreamIndex = i;
			if (st->audioStreamIndex >= 0 && st->subtitleStreamIndex > 0) break;
			// if(st->audioStreamIndex >= 0) break;
		}

		if (st->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && st->audioStreamIndex < 0){
			st->audioStreamIndex = i;
			if (st->videoStreamIndex >= 0 && st->subtitleStreamIndex > 0) break;
			// if(st->videoStreamIndex >= 0) break;
		}

		if (st->pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE && st->subtitleStreamIndex < 0){
			st->subtitleStreamIndex = i;
			if (st->videoStreamIndex >= 0 && st->audioStreamIndex >= 0) break;
		}
	}

	// if(st->videoStreamIndex == -1 || st->audioStreamIndex == -1) return -1;
	if (st->videoStreamIndex == -1) return -1;


	if (st->StreamOpen(st->videoStreamIndex) < 0) return -1;
	// if(st->StreamOpen(st->audioStreamIndex) < 0) return -1;
	if (st->audioStreamIndex != -1 && st->StreamOpen(st->audioStreamIndex) < 0) return -1;
	// if(st->subtitleStreamIndex != -1 && st->StreamOpen(st->subtitleStreamIndex) < 0) return -1;

	if (st->videoContext){
		st->width = st->videoContext->width;
		st->height = st->videoContext->height;
	}

	st->opened = true;

	return 0;
}

void VideoState::Seek(int incr){

	double pos = (GetMasterClock() + incr) * AV_TIME_BASE;

	if (!seekRequest) {
		seekPos = (int)pos;
		seekFlags = (incr * AV_TIME_BASE) < 0 ? AVSEEK_FLAG_BACKWARD : 0;
        seekInc = (incr * AV_TIME_BASE);
		seekRequest = true;
        SDL_CondSignal(continueReadThread);
	}
}

static int DecodeThread(void *data){

	VideoState *st = (VideoState *)data;

	if (OpenState(st) < 0){
		st->quit = true;
		return -1;
	}

    SDL_mutex *waitMutex = SDL_CreateMutex();

	AVPacket pkt1, *packet = &pkt1;

	while (true){

		if (st->quit)
			break;

		if (st->seekRequest) {

			if(st->videoStream){

				int seekPos = (int)av_rescale_q(st->seekPos, { 1, AV_TIME_BASE }, st->pFormatCtx->streams[st->videoStreamIndex]->time_base);

				SDL_LockMutex(st->pictQueueMutex);

				if(av_seek_frame(st->pFormatCtx, st->videoStreamIndex, seekPos, st->seekFlags) >= 0){
					st->videoQueue.Flush();
					st->videoQueue.Put(&st->flushPacket, &st->flushPacket);
				}

				st->vidclk.Set(seekPos);
				st->videoCurrentPtsTime = 0;
				st->pictQueueSize = 0;
				st->pictQueueRIndex = 0;
				st->pictQueueWIndex = 0;

				st->lastFramePts = 0;
				st->lastFrameDelay = 0;
				st->frameTimer = 0;
				st->videoCurrentPts = 0;

				SDL_CondSignal(st->pictQueueCond);

				SDL_UnlockMutex(st->pictQueueMutex);
			}

			if(st->audioStream){

				int seekPos = (int)av_rescale_q(st->seekPos, { 1, AV_TIME_BASE }, st->pFormatCtx->streams[st->audioStreamIndex]->time_base);

				if(av_seek_frame(st->pFormatCtx, st->audioStreamIndex, seekPos, st->seekFlags) >= 0){
					st->audioQueue.Flush();
					st->audioQueue.Put(&st->flushPacket, &st->flushPacket);
				}

				st->audclk.Set(seekPos);

				av_freep(&st->audioBuf1);

				st->audioBuf1Size = st->audioBufSize = st->audioBufIndex = st->audioPacketSize = 0;

				st->audioBuf1 = nullptr;

				st->audioDiffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
				st->audioDiffAvgCount = 0;
				st->audioDiffThreshold = (double)(st->audioHwBufSize) / st->params2.bytes_per_sec;

				memset(&st->audioPacket, 0, sizeof(AVPacket));
			}

			if(st->subtitleStream){

				int seekPos = (int)av_rescale_q(st->seekPos, { 1, AV_TIME_BASE }, st->pFormatCtx->streams[st->subtitleStreamIndex]->time_base);

				if(av_seek_frame(st->pFormatCtx, st->subtitleStreamIndex, seekPos, st->seekFlags) >= 0){
					st->subtitleQueue.Flush();
					st->subtitleQueue.Put(&st->flushPacket, &st->flushPacket);
				}

				SDL_LockMutex(st->subQueueMutex);
				st->subQueueSize = 0;
				st->subPictQueueRIndex = 0;
				st->subPictQueueWIndex = 0;
				SDL_CondSignal(st->subQueueCond);

				SDL_UnlockMutex(st->subQueueMutex);
			}

			st->seekRequest = false;
		}

		if(st->audioQueue.size + st->videoQueue.size + st->subtitleQueue.size > MAX_QUEUE_SIZE ||
			((st->audioQueue.numPackets > MIN_FRAMES || !st->audioStream) &&
			 (st->videoQueue.numPackets > MIN_FRAMES || !st->videoStream))){

        // if(st->audioQueue.size > MAX_AUDIOQ_SIZE){
            SDL_LockMutex(waitMutex);
            SDL_CondWaitTimeout(st->continueReadThread, waitMutex, 10);
            SDL_UnlockMutex(waitMutex);
            continue;
        }

		int ret = av_read_frame(st->pFormatCtx, packet);

		if (ret < 0 || ret == AVERROR_EOF){
			if (st->pFormatCtx->pb->error != 0 || ret == AVERROR_EOF){
				st->quit = true;
				return -1;
			}
		}

		if (packet->stream_index == st->videoStreamIndex){

			st->videoQueue.Put(packet, &st->flushPacket);
		}
		else if (packet->stream_index == st->audioStreamIndex){

			st->audioQueue.Put(packet, &st->flushPacket);
		}
		else if (packet->stream_index == st->subtitleStreamIndex){

			st->subtitleQueue.Put(packet, &st->flushPacket);
		}
		else {

			av_free_packet(packet);
		}
	}

    SDL_DestroyMutex(waitMutex);

	return 0;
}

// void VideoState::CheckExternalClockSpeed() {

//  if ((videoStreamIndex >= 0 && videoQueue.numPackets <= MIN_FRAMES / 2) ||
//      (audioStreamIndex >= 0 && audioQueue.numPackets <= MIN_FRAMES / 2)) {

//      extclk.SetSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));

//  } else if ((videoStreamIndex < 0 || videoQueue.numPackets > MIN_FRAMES * 2) &&

//      (audioStreamIndex < 0 || audioQueue.numPackets > MIN_FRAMES * 2)) {
//      extclk.SetSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));

//  } else {

//      double speed = extclk.speed;

//      if (speed != 1.0)
//          extclk.SetSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
//  }
// }

double VideoState::GetAudioClock(){

	return audclk.Get();
	// return audioClock - (double)(2 * audioHwBufSize + (audioBufSize - audioBufIndex)) / params2.bytes_per_sec;

	// int hw_buf_size, bytes_per_sec, n;

	// double pts = audioClock;
	// hw_buf_size = audioBufSize - audioBufIndex;
	// bytes_per_sec = 0;
	// n = audioContext->channels * 2;

	// if(audioStream)
	//     bytes_per_sec = audioContext->sample_rate * n;

	// if(bytes_per_sec)
	//     pts -= (double)hw_buf_size / bytes_per_sec;

	// return pts;
}

static int VideoThread(void *data){

	if (!data) return 0;

	VideoState *state = (VideoState *)data;

	AVPacket pkt, *packet = &pkt;

	int frameFinished;

	AVFrame *pFrame = av_frame_alloc();

	while (true){

		if (state->quit)
			break;

		if (state->videoQueue.Get(state, packet, true) < 0)
			break;

		if (packet->data == state->flushPacket.data) {
			avcodec_flush_buffers(state->videoStream->codec);
			continue;
		}

		avcodec_decode_video2(state->videoStream->codec, pFrame, &frameFinished, packet);
		pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);

		double pts = pFrame->pts * av_q2d(state->videoStream->time_base);

		if (frameFinished){

			pts = state->SynchronizeVideo(pFrame, pts);
			if (state->QueuePicture(pFrame, pts))
				break;
		}

		av_free_packet(packet);
	}

	av_free(pFrame);

	return 0;
}

static void AudioCallback(void *data, uint8_t *stream, int len){

	VideoState *state = (VideoState *)data;

	int len1, audioSize;

	state->audioCallbackTime = av_gettime_relative();

	while (len > 0){

		if (state->audioBufIndex >= state->audioBufSize){

			audioSize = state->DecodeAudioFrame();

			if (audioSize < 0) {

				state->audioBuf = state->silenceBuf;
				state->audioBufSize = sizeof(state->silenceBuf) / state->params2.frame_size * state->params2.frame_size;

			}
			else {

				state->audioBufSize = audioSize;
			}

			state->audioBufIndex = 0;
		}

		len1 = state->audioBufSize - state->audioBufIndex;

		if (len1 > len) len1 = len;

		memcpy(stream, &state->audioBuf[state->audioBufIndex], len1);
		len -= len1;

		stream += len1;

		state->audioBufIndex += len1;
	}

	state->audioWriteBufSize = state->audioBufSize - state->audioBufIndex;

	if (!isnan(state->audioClock)){

		// state->audclk.SetAt(
		// 	state->audioClock -
		// 	(double)(2 * state->audioHwBufSize + state->audioWriteBufSize) / state->params2.bytes_per_sec, state->audioCallbackTime);

		// printf("%f %f %f\n",state->audioClock, state->audioWriteBufSize, state->audioHwBufSize);

		state->audclk.SetAt(state->audioClock -
			((double)(2 * state->audioHwBufSize + state->audioWriteBufSize) / (double)state->params2.bytes_per_sec),
			state->audioCallbackTime / 1000000.0);

		state->extclk.SetToSlave(&state->audclk);
	}
}

double VideoState::SynchronizeVideo(AVFrame *srcFrame, double pts){

	double frameDelay;

	if (pts != 0){

		videoClock = pts;

	}
	else {

		pts = videoClock;
	}

	frameDelay = av_q2d(videoStream->codec->time_base);
	frameDelay += srcFrame->repeat_pict * (frameDelay * 0.5);
	videoClock += frameDelay;

	return pts;
}

double VideoState::SynchronizeAudio(int numSamples){

	int wanted_nb_samples = numSamples;

	if (avSyncType != AV_SYNC_AUDIO_MASTER) {
		double diff, avg_diff;
		int min_nb_samples, max_nb_samples;

		diff = GetAudioClock() - GetMasterClock();

		if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
			audioDiffCum = diff + audioDiffAvgCoef * audioDiffCum;
			if (audioDiffAvgCount < AUDIO_DIFF_AVG_NB) {

				audioDiffAvgCount++;
			}
			else {

				avg_diff = audioDiffCum * (1.0 - audioDiffAvgCoef);

				if (fabs(avg_diff) >= audioDiffThreshold) {
					wanted_nb_samples = numSamples + (int)(diff * params.freq);
					min_nb_samples = ((numSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					max_nb_samples = ((numSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					wanted_nb_samples = FFMIN(FFMAX(wanted_nb_samples, min_nb_samples), max_nb_samples);
				}
			}
		}
		else {

			audioDiffAvgCount = 0;
			audioDiffCum = 0;
		}
	}

	return wanted_nb_samples;
}

int VideoState::DecodeAudioFrame(){

	int len1 = 0, dataSize = 0, resampledSize = 0;

	while (true){

		while (audioPacketSize > 0){

			int gotFrame = 0;
			len1 = avcodec_decode_audio4(audioContext, &audioFrame, &gotFrame, &audioPacket);

			if (!gotFrame) continue;

			AVRational time_base = { 1, audioFrame.sample_rate };

			if (audioFrame.pts != AV_NOPTS_VALUE)
				audioFrame.pts = av_rescale_q(audioFrame.pts, audioStream->time_base, time_base);
			else if (audioFrame.pkt_pts != AV_NOPTS_VALUE)
				audioFrame.pts = av_rescale_q(audioFrame.pkt_pts, audioStream->time_base, time_base);

			if (len1 < 0){
				audioPacketSize = 0;
				break;
			}

			audioPacketData += len1;
			audioPacketSize -= len1;

			dataSize = av_samples_get_buffer_size(NULL, av_frame_get_channels(&audioFrame),
				audioFrame.nb_samples, (AVSampleFormat)audioFrame.format, 1);

			int64_t dec_channel_layout =
				(audioFrame.channel_layout && av_frame_get_channels(&audioFrame) ==
				av_get_channel_layout_nb_channels(audioFrame.channel_layout)) ?
				audioFrame.channel_layout : av_get_default_channel_layout(av_frame_get_channels(&audioFrame));

			int wanted_nb_samples = (int)SynchronizeAudio(audioFrame.nb_samples);

			if (audioFrame.format != params.fmt ||
				dec_channel_layout != params.channel_layout ||
				audioFrame.sample_rate != params.freq ||

				(wanted_nb_samples != audioFrame.nb_samples && !pSwrCtx)) {

				swr_free(&pSwrCtx);
				pSwrCtx = swr_alloc_set_opts(NULL, params2.channel_layout, params2.fmt, params2.freq,
					dec_channel_layout, (AVSampleFormat)audioFrame.format, audioFrame.sample_rate,
					0, NULL);

				if (!pSwrCtx || swr_init(pSwrCtx) < 0) {

					swr_free(&pSwrCtx);
					return -1;
				}

				params.channel_layout = dec_channel_layout;
				params.channels = av_frame_get_channels(&audioFrame);
				params.freq = audioFrame.sample_rate;
				params.fmt = (AVSampleFormat)audioFrame.format;
			}

			if (pSwrCtx) {

				const uint8_t **in = (const uint8_t **)audioFrame.extended_data;

				int out_count = (int64_t)wanted_nb_samples * params2.freq / audioFrame.sample_rate + 256;
				// int out_count = (int64_t)audioFrame.nb_samples * params2.freq / audioFrame.sample_rate + 256;
				int out_size = av_samples_get_buffer_size(NULL, params2.channels, out_count, params2.fmt, 0);

				if (out_size < 0) return -1;

				if (wanted_nb_samples != audioFrame.nb_samples) {

					if (swr_set_compensation(pSwrCtx, (wanted_nb_samples - audioFrame.nb_samples) * params2.freq / audioFrame.sample_rate,
						wanted_nb_samples * params2.freq / audioFrame.sample_rate) < 0) {

						return -1;
					}
				}

				av_fast_malloc(&audioBuf1, &audioBuf1Size, out_size);
				if (!audioBuf1) return AVERROR(ENOMEM);

				int len2 = swr_convert(pSwrCtx, &audioBuf1, out_count, in, audioFrame.nb_samples);

				if (len2 < 0)  return -1;

				if (len2 == out_count)
					if (swr_init(pSwrCtx) < 0) swr_free(&pSwrCtx);


				audioBuf = audioBuf1;
				// memcpy(&audioBuf[0], audioBuf1, out_size);
				resampledSize = len2 * params2.channels * av_get_bytes_per_sample(params2.fmt);

			}
			else {

				// memcpy(&audioBuf[0],audioFrame.data[0], dataSize);
				audioBuf = audioFrame.data[0];
				resampledSize = dataSize;
			}

			if (resampledSize <= 0) continue;

			if (!isnan((double)audioFrame.pts))
				audioClock = ((double)audioFrame.pts * av_q2d(time_base)) +
				((double)audioFrame.nb_samples / audioFrame.sample_rate);
			else
				audioClock = NAN;

			//static double last_clock = 0;
		//	printf("audio: delay=%0.3f clock=%0.3f\n", audioClock - last_clock, audioClock);
			//last_clock = audioClock;

			return resampledSize;
		}

		if (audioPacket.data) av_free_packet(&audioPacket);

		if (quit) return -1;

		if (audioQueue.Get(this, &audioPacket, 1) < 0)
			return -1;

		if (audioPacket.data == flushPacket.data) {
			avcodec_flush_buffers(audioStream->codec);
			continue;
		}

		audioPacketSize = audioPacket.size;
		audioPacketData = audioPacket.data;

		// if (audioPacket.pts != AV_NOPTS_VALUE)
		// 	audioClock = av_q2d(audioStream->time_base) * audioPacket.pts;

	}

	return 0;
}

int VideoState::QueuePicture(AVFrame *frame, double pts){

	SDL_LockMutex(pictQueueMutex);

	while (pictQueueSize >= VIDEO_PICTURE_QUEUE_SIZE && !quit){

		SDL_CondWait(pictQueueCond, pictQueueMutex);
	}

	SDL_UnlockMutex(pictQueueMutex);

	if (quit) return -1;

	VideoPicture *pic = &pictQueue[pictQueueWIndex];

	if (!pic->data || pic->width != videoStream->codec->width || pic->height != videoStream->codec->height){

		AllocPicture();
	}

	if (pic->data){

		SDL_LockMutex(pictQueueMutex);

		sws_scale(swsCtx, (uint8_t const * const *)frame->data, frame->linesize, 0, videoContext->height,
			pFrameRGB->data, pFrameRGB->linesize);

		memmove(&pic->data[0], pFrameRGB->data[0], pic->width*pic->height * 3);

		if (++pictQueueWIndex == VIDEO_PICTURE_QUEUE_SIZE)
			pictQueueWIndex = 0;

		pictQueueSize++;
		pic->pts = pts;

		SDL_UnlockMutex(pictQueueMutex);

	}

	return 0;
}

void VideoState::VideoDisplay(unsigned int glTexture, std::string &text){

	glBindTexture(GL_TEXTURE_2D, glTexture);


	VideoPicture *pic = &pictQueue[pictQueueRIndex];

	for (int y = 0; y < pic->height / 15; y++){
		for (int x = 0; x < pic->width / 15; x++){
			int index = ((y * pic->height / 15) + x) * 3;
			avgFrameColor.x += pic->data[index];
			avgFrameColor.y += pic->data[index + 1];
			avgFrameColor.z += pic->data[index + 2];
		}
	}

	avgFrameColor /= (float)((pic->width / 15)*(pic->height / 15));

	SDL_LockMutex(pictQueueMutex);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pic->width, pic->height, GL_RGB, GL_UNSIGNED_BYTE, pic->data);

	SDL_UnlockMutex(pictQueueMutex);

	SDL_LockMutex(subQueueMutex);

	if (subtitleStream && subQueueSize > 0){

		SubPicture *sp = &subPictQueue[subPictQueueRIndex];

		if (sp->text.size() > 0){

			text = sp->text;

		}
		else if (sp->sub.format == 0) {

			for (int i = 0; i < (int)sp->sub.num_rects; i++){

				int imgw = (int)pic->width;
				int imgh = (int)pic->height;

				int dstw = (int)av_clip(sp->sub.rects[i]->w, 0, imgw);
				int dsth = (int)av_clip(sp->sub.rects[i]->h, 0, imgh);
				int dstx = (int)av_clip(sp->sub.rects[i]->x, 0, imgw - dstw);
				int dsty = (int)av_clip(sp->sub.rects[i]->y, 0, imgh - dsth);

				glTexSubImage2D(GL_TEXTURE_2D, 0, dstx, dsty, dstw, dsth, GL_RGB, GL_UNSIGNED_BYTE, sp->sub.rects[i]->pict.data);
			}
		}
	}

	SDL_UnlockMutex(subQueueMutex);
}

double VideoState::GetVideoClock(){
	return vidclk.Get();
	// double delta = (av_gettime_relative() - videoCurrentPtsTime) / 1000000.0;
	// return videoCurrentPts + delta;
}

double VideoState::GetExternalClock(){
	return av_gettime_relative() / 1000000.0;
}

double VideoState::GetMasterClock(){
	if (avSyncType == AV_SYNC_VIDEO_MASTER)
		return GetVideoClock();
	else if (avSyncType == AV_SYNC_AUDIO_MASTER)
		return GetAudioClock();
	else
		return GetExternalClock();

	return 0;
}

void VideoState::VideoRefreshTimer(unsigned int glTexture, std::string &text){

	if (!opened || quit) return;

	text = "";

	double actualDelay, delay, syncThreshold, refClock, diff;

	if (videoStream){

		if (pictQueueSize == 0)
			return;

		VideoPicture *pic = &pictQueue[pictQueueRIndex];

		delay = pic->pts - lastFramePts;

		if (avSyncType != AV_SYNC_VIDEO_MASTER) {

			refClock = GetMasterClock();

			diff = pic->pts - refClock;

			syncThreshold = (delay > AV_SYNC_THRESHOLD_MAX) ? delay : AV_SYNC_THRESHOLD_MAX;

			if (fabs(diff) < AV_NOSYNC_THRESHOLD) {
				if (diff <= -syncThreshold) {
					delay = 0;

				}
				else if (diff >= syncThreshold) {
					delay = 2 * delay;
				}
			}
		}

		if(delay < 0 || delay > 1) delay = lastFrameDelay;

		double currTime = (double)av_gettime_relative() / 1000000.0;

		if (currTime < videoCurrentPtsTime + delay)
			return;

		frameTimer += delay;

		SDL_LockMutex(pictQueueMutex);

		videoCurrentPts = pic->pts;
		videoCurrentPtsTime = currTime;
		lastFramePts = pic->pts;
		lastFrameDelay = delay;

		if (!isnan(pic->pts)){
			vidclk.Set(pic->pts);
			extclk.SetToSlave(&vidclk);
		}

		SDL_UnlockMutex(pictQueueMutex);

		if (subtitleStream) {

			while (subQueueSize > 0) {

				SubPicture *sp, *sp2;

				sp = &subPictQueue[subPictQueueRIndex];

				if (subQueueSize > 1)
					sp2 = &subPictQueue[(subPictQueueRIndex + 1) % SUBPICTURE_QUEUE_SIZE];
				else
					sp2 = NULL;

				if ((vidclk.pts > (((float)sp->sub.end_display_time / 1000.0f)))
					|| (sp2 && vidclk.pts > (((float)sp2->sub.start_display_time / 1000.0f)))){

					avsubtitle_free(&sp->sub);

					if (++subPictQueueRIndex == SUBPICTURE_QUEUE_SIZE)
						subPictQueueRIndex = 0;

					SDL_LockMutex(subQueueMutex);
					subQueueSize--;
					SDL_CondSignal(subQueueCond);
					SDL_UnlockMutex(subQueueMutex);
				}
				else {
					break;
				}
			}
		}

		SDL_LockMutex(pictQueueMutex);

		VideoDisplay(glTexture, text);

		if (++pictQueueRIndex == VIDEO_PICTURE_QUEUE_SIZE)
			pictQueueRIndex = 0;

		pictQueueSize--;

		SDL_CondSignal(pictQueueCond);

		SDL_UnlockMutex(pictQueueMutex);
	}
}

void VideoState::AllocPicture(){

	VideoPicture *pic = &pictQueue[pictQueueWIndex];

	SDL_LockMutex(pictQueueMutex);

	if (pic->data) free(pic->data);

	pic->width = videoStream->codec->width;
	pic->height = videoStream->codec->height;
	pic->data = (unsigned char *)malloc(pic->width * pic->height * 3);

	SDL_UnlockMutex(pictQueueMutex);
}

int VideoState::StreamOpen(int streamIndex){

	AVCodecContext *codecContext = pFormatCtx->streams[streamIndex]->codec;
	AVCodec *codec;

	if (streamIndex < 0 || streamIndex >= (int)pFormatCtx->nb_streams) return -1;

	codec = avcodec_find_decoder(codecContext->codec_id);

	AVDictionary *dict = NULL;

	if (!codec || avcodec_open2(codecContext, codec, &dict) < 0) return -1;

	if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO){

		videoStreamIndex = streamIndex;
		videoStream = pFormatCtx->streams[streamIndex];
		videoContext = codecContext;

		frameTimer = (double)av_gettime_relative() / 1000000.0;
		lastFrameDelay = 40e-3;

		videoCurrentPts = 0;
		videoCurrentPtsTime = (double)av_gettime_relative() / 1000000.0;

		vidclk.Init(AV_SYNC_VIDEO_MASTER);

		videoQueue.Init();

		pFrameRGB = av_frame_alloc();

		int numBytes = avpicture_get_size(PIX_FMT_RGB24, videoContext->width, videoContext->height);

		frameBuffer = (uint8_t *)av_malloc(numBytes);

		avpicture_fill((AVPicture *)pFrameRGB, frameBuffer, PIX_FMT_RGB24,
			videoContext->width, videoContext->height);

		swsCtx = sws_getContext(codecContext->width, codecContext->height,
			codecContext->pix_fmt, codecContext->width, codecContext->height,
			PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

		videoTID = SDL_CreateThread(VideoThread, "VideoThread", this);

	}
	else if (codecContext->codec_type == AVMEDIA_TYPE_SUBTITLE){

		subtitleStreamIndex = streamIndex;
		subtitleStream = pFormatCtx->streams[streamIndex];

		subtitleQueue.Init();

		subtitleTID = SDL_CreateThread(SubtitleThread, "SubtitleThread", this);

	}
	else if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO){

		memset(silenceBuf, 0, sizeof(silenceBuf));

		audioStreamIndex = streamIndex;
		audioStream = pFormatCtx->streams[streamIndex];
		audioContext = codecContext;
		audclk.Init(AV_SYNC_AUDIO_MASTER);

		SDL_AudioSpec wanted, spec;

		wanted.freq = codecContext->sample_rate;
		wanted.format = AUDIO_S16SYS;
		wanted.channels = codecContext->channels;
		wanted.silence = 0;
		wanted.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 <<
			av_log2(wanted.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
		wanted.callback = AudioCallback;
		wanted.userdata = this;

		if (SDL_OpenAudio(&wanted, &spec) < 0){
			printf("%s\n", SDL_GetError());
			return -1;
		}

		int wantedChannels = audioContext->channels;
		int wantedChannelLayout = (int)audioContext->channel_layout;

		if (audioContext->channel_layout == 0 && audioContext->channels == 2) {
			wantedChannelLayout = AV_CH_LAYOUT_STEREO;

		}
		else if (audioContext->channel_layout == 0 && audioContext->channels == 1) {
			wantedChannelLayout = AV_CH_LAYOUT_MONO;

		}
		else if (audioContext->channel_layout == 0 && audioContext->channels == 0) {
			wantedChannelLayout = AV_CH_LAYOUT_STEREO;
			wantedChannels = 2;
		}

		if (codecContext->sample_fmt != AV_SAMPLE_FMT_S16)
			audioNeedsResampling = true;

		audioHwBufSize = spec.size;

		params.fmt = AV_SAMPLE_FMT_S16;
		params.freq = spec.freq;
		params.channel_layout = wantedChannelLayout;
		params.channels = wantedChannels;
		params.frame_size = av_samples_get_buffer_size(NULL, params.channels, 1, params.fmt, 1);
		params.bytes_per_sec = av_samples_get_buffer_size(NULL, params.channels, params.freq, params.fmt, 1);

		params2 = params;

		audioBuf1Size = audioBufSize = audioBufIndex = audioPacketSize = 0;

		audioBuf1 = nullptr;

		audioDiffAvgCoef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
		audioDiffAvgCount = 0;
		audioDiffThreshold = (double)(audioHwBufSize) / params2.bytes_per_sec;

		memset(&audioPacket, 0, sizeof(AVPacket));

		audioQueue.Init();

		SDL_PauseAudio(0);
	}

	return 0;
}

int VideoState::Init(const char *filePath, bool isStr, const std::string &command, const std::string &cookies){

	if (!quit || opened) return -1;

	this->isStream = isStr;

	quit = false;

	strcpy_s(fileName, filePath);

	this->cookies = cookies;
	this->command = command;

	pictQueueMutex = SDL_CreateMutex();
	pictQueueCond = SDL_CreateCond();
	subQueueMutex = SDL_CreateMutex();
	subQueueCond = SDL_CreateCond();
    continueReadThread = SDL_CreateCond();

	audclk.Init(AV_SYNC_AUDIO_MASTER);
	vidclk.Init(AV_SYNC_VIDEO_MASTER);
	extclk.Init(AV_SYNC_EXTERNAL_MASTER);

	av_init_packet(&flushPacket);
	flushPacket.data = (unsigned char *)"FLUSH";

	parseTID = SDL_CreateThread(DecodeThread, "ParseThread", this);

	return 0;
}

void VideoState::Update(){
	if (quit && opened)
		Close();
}

void VideoState::Close(){

	if (quit && !opened) return;

	quit = true;
	opened = false;

	SDL_CloseAudio();

	SDL_CondSignal(videoQueue.cond);
	if (subtitleStreamIndex) SDL_CondSignal(subtitleQueue.cond);
	SDL_CondSignal(audioQueue.cond);
	SDL_CondSignal(pictQueueCond);
	SDL_CondSignal(subQueueCond);

	int status;

	SDL_WaitThread(parseTID, &status);
	SDL_WaitThread(videoTID, &status);
	if (subtitleStreamIndex) SDL_WaitThread(subtitleTID, &status);

	SDL_DestroyCond(continueReadThread);

	SDL_DestroyCond(pictQueueCond);
	SDL_DestroyMutex(pictQueueMutex);

	SDL_DestroyCond(subQueueCond);
	SDL_DestroyMutex(subQueueMutex);

	videoQueue.Destroy();
	avcodec_close(videoContext);

	audioQueue.Destroy();
	avcodec_close(audioContext);

	subtitleQueue.Destroy();
	avcodec_close(subtitleContext);

	avformat_close_input(&pFormatCtx);
	if (pFrameRGB) av_free(pFrameRGB);
	if (frameBuffer) av_free(frameBuffer);
	if (swsCtx) sws_freeContext(swsCtx);

	av_freep(&audioBuf1);
	audioBufSize = 0;
	audioBuf = NULL;
}
