#ifndef VIDEO_STATE_DEF
#define VIDEO_STATE_DEF

#include "ymath.h"

extern "C"{
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

// #define MAX_QUEUE_SIZE (15 * 1024 * 1024)
// #define MIN_FRAMES 5

#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20
#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_AUDIO_MASTER
#define SUBPICTURE_QUEUE_SIZE 16

#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))


#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5

// #define SDL_AUDIO_MIN_BUFFER_SIZE 512
// /* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
// #define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

// /* no AV sync correction is done if below the minimum AV sync threshold */
// #define AV_SYNC_THRESHOLD_MIN 0.04
// /* AV sync correction is done if above the maximum AV sync threshold */
// #define AV_SYNC_THRESHOLD_MAX 0.1
// /* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
// #define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
// /* no AV correction is done if too big error */
// #define AV_NOSYNC_THRESHOLD 10.0

// /* maximum audio speed change to get correct sync */
// #define SAMPLE_CORRECTION_PERCENT_MAX 10

// /* external clock speed adjustment constants for realtime sources based on buffer fullness */
// #define EXTERNAL_CLOCK_SPEED_MIN  0.900
// #define EXTERNAL_CLOCK_SPEED_MAX  1.010
// #define EXTERNAL_CLOCK_SPEED_STEP 0.001

// /* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
// #define AUDIO_DIFF_AVG_NB   20

// /* polls for possible required screen refresh at least this often, should be less than 1/fps */
// #define REFRESH_RATE 0.01

// /* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
// /* TODO: We assume that a decoded and resampled frame fits into this buffer */
// #define SAMPLE_ARRAY_SIZE (8 * 65536)

// #define CURSOR_HIDE_DELAY 1000000

// #define VIDEO_PICTURE_QUEUE_SIZE 3
// #define SUBPICTURE_QUEUE_SIZE 16
// #define SAMPLE_QUEUE_SIZE 9
// #define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

// #define SDL_AUDIO_BUFFER_SIZE 1024
// #define MAX_AUDIO_FRAME_SIZE 192000

class VideoState;

class PacketQueue {
public:

	PacketQueue() :
		firstPacket(nullptr),
		lastPacket(nullptr),
		numPackets(0),
		size(0),
		mutex(nullptr),
		cond(nullptr)
	{}

	AVPacketList *firstPacket, *lastPacket;
	int numPackets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;

	void Init();
	void Flush();
	void Destroy();
	int Put(AVPacket *packet, AVPacket *flushPacket);
	int Get(VideoState *vState, AVPacket *packet, bool block);

private:
};

typedef struct Frame {
	AVFrame *frame;
	AVSubtitle sub;
	int serial;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	char *data;
	int allocated;
	int reallocate;
	int width;
	int height;
	AVRational sar;
} Frame;

typedef struct {
	unsigned char *data;
	int width;
	int height;
	double pts;
} VideoPicture;

typedef struct {
	int freq;
	int channels;
	int64_t channel_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

enum {
	AV_SYNC_AUDIO_MASTER,
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_MASTER,
};

class Clock {
public:

	Clock() : pts(0), ptsDrift(0), lastUpdated(0), speed(0), type(0), paused(0){}

	void Init(int);
	void Set(double);
	void SetAt(double, double);
	double Get();
	void SetToSlave(Clock *);
	void SetSpeed(double speed);

	double pts;           /* clock base */
	double ptsDrift;     /* clock base minus time at which we updated the clock */
	double lastUpdated;
	double speed;
	int type;
	int paused;
};

typedef struct SubPicture {
	double pts;
	AVSubtitle sub;
	int serial;
	std::string text;
} SubPicture;

class VideoState {
public:

	VideoState() :
		isStream(false),
		opened(false),
		seekInc(0),
		width(0),
		height(0),
		seekPos(0),
		seekRequest(0),
		seekFlags(0),
		pFormatCtx(nullptr),
		videoStreamIndex(-1),
		audioStreamIndex(-1),
		subtitleStreamIndex(-1),
		audioStream(nullptr),
		subtitleStream(nullptr),
		audioBuf1(nullptr),
		audioBufSize(0),
		audioBufIndex(0),
		audioPacketData(nullptr),
		audioPacketSize(0),
		videoContext(nullptr),
		audioContext(nullptr),
		subtitleContext(nullptr),
		videoStream(nullptr),
		pictQueueSize(0),
		pictQueueRIndex(0),
		pictQueueWIndex(0),
		pictQueueMutex(nullptr),
		pictQueueCond(nullptr),
		subQueueMutex(nullptr),
		subQueueCond(nullptr),
		continueReadThread(nullptr),
		videoTID(nullptr),
		parseTID(nullptr),
		pFrameRGB(nullptr),
		subQueueSize(0),
		subPictQueueRIndex(0),
		subPictQueueWIndex(0),
		frameBuffer(nullptr),
		quit(true),
		swsCtx(nullptr),
		pSwrCtx(nullptr),
		audioNeedsResampling(false),
		pResampledOut(nullptr),
		resampleLines(0),
		resampleSize(0),
		lastFramePts(0),
		lastFrameDelay(0),
		frameTimer(0),
		avSyncType(AV_SYNC_AUDIO_MASTER),
		audioDiffCum(0),
		audioDiffAvgCoef(0),
		audioDiffThreshold(0),
		audioDiffAvgCount(0),
		audioBuf1Size(0),
		audioCallbackTime(0),
		audioHwBufSize(0),
		audioWriteBufSize(0),
		videoCurrentPts(0),
		videoCurrentPtsTime(0)
	{}

	int StreamOpen(int streamIndex);
	int DecodeAudioFrame();
	int Init(const char *filePath, bool isStream = false, const std::string &command = "", const std::string &cookies = "");
	void Close();
	void AllocPicture();
	int QueuePicture(AVFrame *frame, double pts);
	void VideoDisplay(unsigned int glTexture, std::string &text);
	void VideoRefreshTimer(unsigned int glTexture, std::string &text);
	double SynchronizeVideo(AVFrame *srcFrame, double pts);
	double SynchronizeAudio(int numSamples);
	void CheckExternalClockSpeed();
	double GetVideoClock();
	double GetAudioClock();
	double GetExternalClock();
	int AddNeededSamples(short *samples, int samplesSize, int wantedSize);
	void Update();
	void Seek(int incr);
	double GetMasterClock();

	bool 				isStream;
	bool 				opened;
	std::string 		cookies;
	std::string 		command;

	int 				seekInc;
	int 				width;
	int 				height;

	int 				seekPos;
	bool 				seekRequest;
	int 				seekFlags;

	AVPacket 			flushPacket;

	double 				audioClock;
	double 				videoClock;
	AVFormatContext 	*pFormatCtx;
	int 				videoStreamIndex;
	int 				audioStreamIndex;
	int 				subtitleStreamIndex;
	AVStream        	*audioStream;
	AVStream        	*subtitleStream;
	PacketQueue 		subtitleQueue;
	PacketQueue 		audioQueue;

	uint8_t 			*audioBuf;
	uint8_t         	*audioBuf1;
	unsigned int    	audioBufSize;
	unsigned int    	audioBufIndex;
	AVFrame         	audioFrame;
	AVPacket        	audioPacket;
	uint8_t         	*audioPacketData;
	int             	audioPacketSize;
	uint8_t 			silenceBuf[512];

	AVCodecContext 		*videoContext;
	AVCodecContext 		*audioContext;
	AVCodecContext		*subtitleContext;


	AVStream 			*videoStream;
	PacketQueue 		videoQueue;
	VideoPicture 		pictQueue[VIDEO_PICTURE_QUEUE_SIZE];
	int 				pictQueueSize;
	int 				pictQueueRIndex;
	int 				pictQueueWIndex;
	SDL_mutex 			*pictQueueMutex;
	SDL_cond 			*pictQueueCond;
	SDL_mutex 			*subQueueMutex;
	SDL_cond 			*subQueueCond;
	SDL_cond 			*continueReadThread;
	SDL_Thread 			*videoTID;
	SDL_Thread 			*parseTID;
	SDL_Thread 			*subtitleTID;
	AVFrame 			*pFrameRGB;

	SubPicture 			subPictQueue[SUBPICTURE_QUEUE_SIZE];
	int 				subQueueSize;
	int 				subPictQueueRIndex;
	int 				subPictQueueWIndex;

	uint8_t 			*frameBuffer;
	char 				fileName[1024];
	bool 				quit;

	struct SwsContext 	*swsCtx;
	SwrContext 			*pSwrCtx;

	bool 				audioNeedsResampling;
	uint8_t 			*pResampledOut;
	int 				resampleLines;
	uint64_t 			resampleSize;

	AudioParams 		params;
	AudioParams 		params2;

	double lastFramePts;
	double lastFrameDelay;
	double frameTimer;

	int avSyncType;

	double audioDiffCum;
	double audioDiffAvgCoef;
	double audioDiffThreshold;
	double audioDiffAvgCount;
	unsigned int audioBuf1Size;
	double audioCallbackTime;
	double audioHwBufSize;
	double audioWriteBufSize;

	double videoCurrentPts;
	double videoCurrentPtsTime;

	Vector3 avgFrameColor;

	Clock audclk;
	Clock vidclk;
	Clock extclk;

	static const std::string CookiesFile;
};

#endif