/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxHapPlayer.h"
#include "ofxOsc.h"
#include "ofxSyphon.h"

#include "sjCommon.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class OSC_SEND{
private:
	char IP[BUF_SIZE];
	int Port;

	ofxOscSender sender;
	
public:
	OSC_SEND()
	: Port(-1)
	{
	}
	OSC_SEND(const char* _IP, int _Port)
	{
		if(_Port != -1){
			sprintf(IP, "%s", _IP);
			Port = _Port;
			
			sender.setup(IP, Port);
		}
	}
	
	void setup(const char* _IP, int _Port)
	{
		if(_Port != -1){
			sprintf(IP, "%s", _IP);
			Port = _Port;
			
			sender.setup(IP, Port);
		}
	}
	void sendMessage(ofxOscMessage &message)
	{
		if(Port != -1){
			sender.sendMessage(message);
		}
	}
};

class OSC_RECEIVE{
private:
	int Port;
	ofxOscReceiver	receiver;
	
public:
	OSC_RECEIVE()
	: Port(-1)
	{
	}
	OSC_RECEIVE(int _Port)
	{
		if(_Port != -1){
			Port = _Port;
			receiver.setup(Port);
		}
	}
	
	void setup(int _Port)
	{
		if(_Port != -1){
			Port = _Port;
			receiver.setup(Port);
		}
	}
	
	bool hasWaitingMessages()
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.hasWaitingMessages();
		}
	}
	
	bool getNextMessage(ofxOscMessage *msg)
	{
		if(Port == -1){
			return false;
		}else{
			return receiver.getNextMessage(msg);
		}
	}
};

class OSC_TARGET{
private:
public:
	OSC_SEND	OscSend;
	OSC_RECEIVE	OscReceive;
	
	OSC_TARGET()
	{
	}
	
	OSC_TARGET(const char* _SendTo_IP, int _SendTo_Port, int _Receive_Port)
	: OscSend(_SendTo_IP, _SendTo_Port), OscReceive(_Receive_Port)
	{
	}
	
	void setup(const char* _SendTo_IP, int _SendTo_Port, int _Receive_Port)
	{
		OscSend.setup(_SendTo_IP, _SendTo_Port);
		OscReceive.setup(_Receive_Port);
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		MONITOR_WIDTH = 320,
		MONITOR_HEIGHT = 180,
	};
	enum{
		VIDEO_WIDTH = 1280,
		VIDEO_HEIGHT = 720,
	};
	enum{
		NUM_VIDEOS = 3,
	};
	
	/****************************************
	****************************************/
	struct TABLE_MOV_INFO{
		string FileName;
		int BeatInterval_ms;
	};
	
	/****************************************
	param
	****************************************/
	/********************
	********************/
	OSC_TARGET Osc_VJ;
	int ServerId;
	
	ofxSyphonServer fbo_TextureSyphonServer[NUM_VIDEOS];
	ofFbo fbo[NUM_VIDEOS];
	ofxHapPlayer video[NUM_VIDEOS];
	
	/********************
	********************/
	char path_mov0[BUF_SIZE];
	char path_mov12[BUF_SIZE];
	
	vector<TABLE_MOV_INFO> Table_mov0;
	vector<TABLE_MOV_INFO> Table_mov12;
	
	int id_mov_0;
	int id_mov_12;
	
	float t_LastMessage_ContentsChange;
	
	int dispVideo_id;
	
	bool b_test_ContentsChagne;
	bool b_monitor;
	
	
	/****************************************
	****************************************/
	void ReadConfig();
	void makeup_mov_table(const string dirname, vector<TABLE_MOV_INFO>& Table_mov);
	void shuffle_TableMov(vector<TABLE_MOV_INFO>& Table_mov);
	void setup_video(ofxHapPlayer& video);
	int getNextId_Table_mov(vector<TABLE_MOV_INFO>& Table_mov, int& id);
	void ChangeVideoContents();
	
public:
	/****************************************
	****************************************/
	ofApp();
	~ofApp();
	
	void exit();
	
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};
