/************************************************************
************************************************************/
#include "ofApp.h"

#include <time.h>

/* for dir search */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>
#include <string>

using namespace std;

/* */


/************************************************************
param
************************************************************/


/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp()
: t_LastMessage_ContentsChange(0)
, dispVideo_id(0)
, b_test_ContentsChagne(false)
, b_monitor(true)
, Video_FadeInterval_Frames(10)
{
}

/******************************
******************************/
ofApp::~ofApp()
{
}

/******************************
******************************/
void ofApp::exit()
{
	printMessage("Good-bye");
}

/******************************
******************************/
void ofApp::ReadConfig()
{
	/********************
	********************/
	char OscIP_SendTo[BUF_SIZE];
	int OscPort_SendTo;
	int OscPost_Receive;
	
	FILE* fp;
	fp = fopen("../../../data/config.txt", "r");
	if(fp == NULL) { ERROR_MSG(); std::exit(1); }
	
	char buf[BUF_SIZE];
	while(fscanf(fp, "%s", buf) != EOF){
		if(strcmp(buf, "<VJ_IP>") == 0){
			fscanf(fp, "%s", buf);
			strcpy(OscIP_SendTo, buf);
			
		}else if(strcmp(buf, "<VJ_port>") == 0){
			fscanf(fp, "%s", buf);
			OscPort_SendTo = atoi(buf);
			
		}else if(strcmp(buf, "<This_port>") == 0){
			fscanf(fp, "%s", buf);
			OscPost_Receive = atoi(buf);
			
		}else if(strcmp(buf, "<ServerId>") == 0){
			fscanf(fp, "%s", buf);
			ServerId = atoi(buf);
			
		}else if(strcmp(buf, "<mov_0>") == 0){
			/********************
			スキャン集合
				http://wisdom.sakura.ne.jp/programming/c/c58.html
			********************/
			fscanf(fp, "%[ \t]", buf); // space & tab 読み捨て
			fscanf(fp, "%[^\n]", buf); // \n以外を読み取る -> \nが来るまで読み込み(space also)
			sprintf(path_mov0, "%s", buf);
			
		}else if(strcmp(buf, "<mov_12>") == 0){
			/********************
			スキャン集合
				http://wisdom.sakura.ne.jp/programming/c/c58.html
			********************/
			fscanf(fp, "%[ \t]", buf); // space & tab 読み捨て
			fscanf(fp, "%[^\n]", buf); // \n以外を読み取る -> \nが来るまで読み込み(space also)
			sprintf(path_mov12, "%s", buf);
		}
	}
	
	fclose(fp);
	
	/********************
	********************/
	printMessage("config data");
	printf("server id = %d,  OscIP_SendTo = %s, OscPort_SendTo = %d, OscPort_Receive = %d\n", ServerId, OscIP_SendTo, OscPort_SendTo, OscPost_Receive);
	printf("path_mov0 :%s\n", path_mov0);
	printf("path_mov12:%s\n", path_mov12);
	
	Osc_VJ.setup(OscIP_SendTo, OscPort_SendTo, OscPost_Receive);
}

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	srand((unsigned) time(NULL));
	
	/********************
	********************/
	ReadConfig();
	
	/********************
	********************/
	char buf[BUF_SIZE];
	sprintf(buf, "VideoServer %d", ServerId);
	ofSetWindowTitle( buf );
	
	ofSetWindowShape( MONITOR_WIDTH, MONITOR_HEIGHT );
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetEscapeQuitsApp(false);

	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		sprintf(buf, "Server_%d_%d", ServerId, i);
		fbo_TextureSyphonServer[i].setName(buf);
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		fbo[i].allocate(VIDEO_WIDTH, VIDEO_HEIGHT);
	}
	
	/********************
	********************/
	/*
	makeup_mov_table("../../../data/mov_0", Table_mov0);
	makeup_mov_table("../../../data/mov_12", Table_mov12);
	
	// makeup_mov_table("/Users/nobuhiro/Documents/source/openframeworks/data/vj\ material/vj\ Hap\(1280x720\)/mov_0", Table_mov0);
	// makeup_mov_table("/Users/nobuhiro/Documents/source/openframeworks/data/vj\ material/vj\ Hap\(1280x720\)/mov_12", Table_mov12);
	*/
	
	makeup_mov_table(path_mov0, Table_mov0);
	makeup_mov_table(path_mov12, Table_mov12);

	
	shuffle_TableMov(Table_mov0);
	shuffle_TableMov(Table_mov12);
	
	id_mov_0 = 0;
	id_mov_12 = 0;
	
	/********************
	********************/
	printMessage("Load_0");
	printf("%s\n", Table_mov0[id_mov_0].FileName.c_str());
	video[0].load(Table_mov0[id_mov_0].FileName.c_str());
	setup_video(video[0]);
	
	printMessage("Load_1");
	printf("%s\n", Table_mov12[id_mov_12].FileName.c_str());
	video[1].load(Table_mov12[id_mov_12].FileName.c_str());
	setup_video(video[1]);
	
	id_mov_12 = getNextId_Table_mov(Table_mov12, id_mov_12);
	printMessage("Load_2");
	printf("%s\n", Table_mov12[id_mov_12].FileName.c_str());
	video[2].load(Table_mov12[id_mov_12].FileName.c_str());
	setup_video(video[2]);
	
	/********************
	********************/
	ofxOscMessage m_send;
	m_send.setAddress("/Ready_CallBack");
	m_send.addIntArg(1);
	Osc_VJ.OscSend.sendMessage(m_send);
}

/******************************
******************************/
int ofApp::getNextId_Table_mov(vector<TABLE_MOV_INFO>& Table_mov, int& id)
{
	id++;
	if(Table_mov.size() <= id){
		id = 0;
		shuffle_TableMov(Table_mov);
		
		printMessage("shuffle TableMov");
	}
	
	return id;
}

/******************************
******************************/
void ofApp::setup_video(ofxHapPlayer& video)
{
	video.setLoopState(OF_LOOP_NORMAL);
	// video.setLoopState(OF_LOOP_PALINDROME);
	
	video.setSpeed(1);
	video.setVolume(0.0);
	video.play();
}

/******************************
description
	fisher yates法
	偏りをなくすため、回を追うごとに乱数範囲を狭めていくのがコツ
	http://www.fumiononaka.com/TechNotes/Flash/FN0212003.html
******************************/
void ofApp::shuffle_TableMov(vector<TABLE_MOV_INFO>& Table_mov)
{
	int i = Table_mov.size();
	
	while(i--){
		int j = rand() % (i + 1);
		
		TABLE_MOV_INFO temp = Table_mov[i];
		Table_mov[i] = Table_mov[j];
		Table_mov[j] = temp;
	}
}

/******************************
******************************/
void ofApp::makeup_mov_table(const string dirname, vector<TABLE_MOV_INFO>& Table_mov)
{
	/********************
	********************/
	DIR *pDir;
	struct dirent *pEnt;
	struct stat wStat;
	string wPathName;

	pDir = opendir( dirname.c_str() );
	if ( NULL == pDir ) { ERROR_MSG(); std::exit(1); }

	pEnt = readdir( pDir );
	while ( pEnt ) {
		// .と..は処理しない
		if ( strcmp( pEnt->d_name, "." ) && strcmp( pEnt->d_name, ".." ) ) {
		
			wPathName = dirname + "/" + pEnt->d_name;
			
			// ファイルの情報を取得
			if ( stat( wPathName.c_str(), &wStat ) ) {
				printf( "Failed to get stat %s \n", wPathName.c_str() );
				break;
			}
			
			if ( S_ISDIR( wStat.st_mode ) ) {
				// nothing.
			} else {
			
				vector<string> str = ofSplitString(pEnt->d_name, ".");
				if(str[str.size()-1] == "mov"){
					TABLE_MOV_INFO Table_NewVal;
					
					Table_NewVal.FileName = wPathName;
					
					vector<string> str2 = ofSplitString(pEnt->d_name, "#");
					if(str2.size() == 3){
						Table_NewVal.BeatInterval_ms = atoi(str2[1].c_str());
					}else{
						Table_NewVal.BeatInterval_ms = -1;
					}
					
					Table_mov.push_back(Table_NewVal);
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(Table_mov.size() < 3){
		ERROR_MSG();
		std::exit(1);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	float ElapsedTime_f = ofGetElapsedTimef();

	/********************
	********************/
	while(Osc_VJ.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc_VJ.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/VJContentsChange"){
			/********************
			********************/
			m_receive.getArgAsInt32(0); /* 読み捨て*/
			
			ChangeVideoContents();
			
			t_LastMessage_ContentsChange = ElapsedTime_f;
			
		}else if(m_receive.getAddress() == "/Quit"){
			std::exit(1);
		}
	}
	
	/********************
	********************/
	if(b_test_ContentsChagne){
		b_test_ContentsChagne = false;
		ChangeVideoContents();
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		video[i].update();
	}
	
	/********************
	********************/
	if(1.0 < ElapsedTime_f - t_LastMessage_ContentsChange){
		ofxOscMessage m_send;
		m_send.setAddress("/Ready_CallBack");
		m_send.addIntArg(1);
		Osc_VJ.OscSend.sendMessage(m_send);
		
		t_LastMessage_ContentsChange = ElapsedTime_f;
	}
}

/******************************
******************************/
void ofApp::ChangeVideoContents()
{
	/********************
	********************/
	dispVideo_id = 0;
	
	/********************
	********************/
	printMessage("Contents Change");
	for(int i = 0; i < NUM_VIDEOS; i++){
		video[i].stop();
		video[i].close();
	}
	
	id_mov_0 = getNextId_Table_mov(Table_mov0, id_mov_0);
	printf("%s\n", Table_mov0[id_mov_0].FileName.c_str());
	video[0].load(Table_mov0[id_mov_0].FileName.c_str());
	setup_video(video[0]);
	
	id_mov_12 = getNextId_Table_mov(Table_mov12, id_mov_12);
	printf("%s\n", Table_mov12[id_mov_12].FileName.c_str());
	video[1].load(Table_mov12[id_mov_12].FileName.c_str());
	setup_video(video[1]);

	id_mov_12 = getNextId_Table_mov(Table_mov12, id_mov_12);
	printf("%s\n", Table_mov12[id_mov_12].FileName.c_str());
	video[2].load(Table_mov12[id_mov_12].FileName.c_str());
	setup_video(video[2]);
	
	printf("Finish loading\n");
	
	/********************
	********************/
	ofxOscMessage m_send;
	m_send.setAddress("/Ready_CallBack");
	m_send.addIntArg(1);
	Osc_VJ.OscSend.sendMessage(m_send);
}

//--------------------------------------------------------------
void ofApp::draw(){
	/********************
	********************/
	// ofBackground(0);
	
	/********************
	********************/
	// Clear with alpha, so we can capture via syphon and composite elsewhere should we want.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		fbo[i].begin();
		ofBackground(0);
		
		float alpha = 1.0;
		int TotalFrames = video[i].getTotalNumFrames();
		int CurrentFrame = video[i].getCurrentFrame();
		if(CurrentFrame < Video_FadeInterval_Frames){
			alpha = 1.0 / Video_FadeInterval_Frames * CurrentFrame;
		}else if(TotalFrames - Video_FadeInterval_Frames < CurrentFrame){
			alpha = 1 - 1.0/Video_FadeInterval_Frames * (Video_FadeInterval_Frames - (TotalFrames - CurrentFrame));
		}
		ofSetColor(255, 255, 255, int(255 * alpha));
		
		video[i].draw(0, 0, fbo[i].getWidth(), fbo[i].getHeight());
		fbo[i].end();
		
		ofTexture tex = fbo[i].getTextureReference();
		fbo_TextureSyphonServer[i].publishTexture(&tex);
	}
	
	/********************
	********************/
	if(b_monitor) fbo[dispVideo_id].draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case '0':
		case '1':
		case '2':
			dispVideo_id = key - '0';
			break;
			
		case 'm':
			b_monitor = !b_monitor;
			break;
			
		case 'c':
			b_test_ContentsChagne = true;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
