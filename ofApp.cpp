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

, Video_FadeInterval_Frames(10)
// , Video_FadeInterval_Frames(0)

, State(STATE_STOP)
, k_PLAY(false)
, k_STOP(false)
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
			sprintf(path_mov[0], "%s", buf);
			
		}else if(strcmp(buf, "<mov_1>") == 0){
			fscanf(fp, "%[ \t]", buf); // space & tab 読み捨て
			fscanf(fp, "%[^\n]", buf); // \n以外を読み取る -> \nが来るまで読み込み(space also)
			sprintf(path_mov[1], "%s", buf);
			
		}else if(strcmp(buf, "<mov_2>") == 0){
			fscanf(fp, "%[ \t]", buf); // space & tab 読み捨て
			fscanf(fp, "%[^\n]", buf); // \n以外を読み取る -> \nが来るまで読み込み(space also)
			sprintf(path_mov[2], "%s", buf);
		}
	}
	
	fclose(fp);
	
	/********************
	********************/
	printMessage("config data");
	printf("server id = %d,  OscIP_SendTo = %s, OscPort_SendTo = %d, OscPort_Receive = %d\n", ServerId, OscIP_SendTo, OscPort_SendTo, OscPost_Receive);
	for(int i = 0; i < NUM_VIDEOS; i++){
		printf("path_mov%d:%s\n", i, path_mov[i]);
	}
	
	Osc_VJ.setup(OscIP_SendTo, OscPort_SendTo, OscPost_Receive);
}

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	srand((unsigned) time(NULL));
	
	font.loadFont("FTY DELIRIUM NCV.ttf", 150);
	
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
}

/******************************
******************************/
void ofApp::Process_STOP_to_PLAY()
{
	/********************
	********************/
	printMessage("PLAY start");
	
	/********************
	********************/
	/*
	makeup_mov_table("../../../data/mov_0", Table_mov0);
	makeup_mov_table("../../../data/mov_12", Table_mov12);
	
	// makeup_mov_table("/Users/nobuhiro/Documents/source/openframeworks/data/vj\ material/vj\ Hap\(1280x720\)/mov_0", Table_mov0);
	// makeup_mov_table("/Users/nobuhiro/Documents/source/openframeworks/data/vj\ material/vj\ Hap\(1280x720\)/mov_12", Table_mov12);
	*/
	
	for(int i = 0; i < NUM_VIDEOS; i++){
		makeup_mov_table(path_mov[i], Table_mov[i]);
		shuffle_TableMov(Table_mov[i]);
		id_mov[i] = 0;
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		char buf[BUF_SIZE];
		sprintf(buf, "Load_%d", i);
		printMessage(buf);
		
		printf("%s\n", Table_mov[i][id_mov[i]].FileName.c_str());
		video[i].load(Table_mov[i][id_mov[i]].FileName.c_str());
		setup_video(video[i]);
	}
	
	/********************
	********************/
	ofxOscMessage m_send;
	m_send.setAddress("/Ready_CallBack");
	m_send.addIntArg(1);
	Osc_VJ.OscSend.sendMessage(m_send);
	
	t_LastMessage_ContentsChange = ofGetElapsedTimef();
	
	/********************
	********************/
	State = STATE_PLAY;
	printf("\nFinish Process\n");
}

/******************************
******************************/
void ofApp::Process_PLAY_to_STOP()
{
	/********************
	********************/
	printMessage("STOP");

	/********************
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		video[i].stop();
		video[i].close();
	}
	
	/********************
	C++ STL vectorのメモリ解放
		http://nonbiri-tereka.hatenablog.com/entry/2014/06/25/164019
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		vector<TABLE_MOV_INFO>().swap(Table_mov[i]);
	}
	
	/********************
	********************/
	State = STATE_STOP;
	printf("Finish Process\n");
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
	if( (State == STATE_STOP) && (k_PLAY) ){
		k_PLAY = false;
		Process_STOP_to_PLAY();
		
	}else if( (State == STATE_PLAY) && (k_STOP) ){
		k_STOP = false;
		Process_PLAY_to_STOP();
		
	}
	
	/********************
	********************/
	while(Osc_VJ.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc_VJ.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/VJContentsChange"){
			/********************
			********************/
			m_receive.getArgAsInt32(0); /* 読み捨て*/
			
			if(State == STATE_PLAY){
				ChangeVideoContents();
				t_LastMessage_ContentsChange = ElapsedTime_f;
			}
			
		}else if(m_receive.getAddress() == "/Quit"){
			std::exit(1);
		}
	}
	
	/********************
	********************/
	if(b_test_ContentsChagne){
		b_test_ContentsChagne = false;
		
		if(State == STATE_PLAY) ChangeVideoContents();
	}
	
	/********************
	********************/
	if(State == STATE_PLAY){
		/* */
		for(int i = 0; i < NUM_VIDEOS; i++){
			video[i].update();
		}
		
		/* */
		if(1.0 < ElapsedTime_f - t_LastMessage_ContentsChange){
			ofxOscMessage m_send;
			m_send.setAddress("/Ready_CallBack");
			m_send.addIntArg(1);
			Osc_VJ.OscSend.sendMessage(m_send);
			
			t_LastMessage_ContentsChange = ElapsedTime_f;
		}
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
	
	for(int i = 0; i < NUM_VIDEOS; i++){
		id_mov[i] = getNextId_Table_mov(Table_mov[i], id_mov[i]);
		printf("%s\n", Table_mov[i][id_mov[i]].FileName.c_str());
		video[i].load(Table_mov[i][id_mov[i]].FileName.c_str());
		setup_video(video[i]);
	}
	
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
	// Clear with alpha, so we can capture via syphon and composite elsewhere should we want.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/********************
	rise/fallにfadeを挿入した.
	->Loop素材でなくても自然にLoopさせるため
	
	ただし、元movieのcodecで、Hap codec(not High Quality) とすること.
	High Qualityだと、αが効かず、常にonとなってしまった(実験結果).
		ffmpeg -i test.mp4 -vcodec hap -format hap   out.mov 
	********************/
	for(int i = 0; i < NUM_VIDEOS; i++){
		fbo[i].begin();
			ofBackground(0);
			
			if(State == STATE_PLAY){
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
				
			}else{
				char DispMessage[BUF_SIZE];
				sprintf(DispMessage, "STOP:[%d]", i);
				
				float offset_x = font.stringWidth(DispMessage) / 2;
				font.drawString(DispMessage, VIDEO_WIDTH/2 - offset_x, VIDEO_HEIGHT/2);
				
			}
		fbo[i].end();
		
		ofTexture tex = fbo[i].getTextureReference();
		fbo_TextureSyphonServer[i].publishTexture(&tex);
	}
	
	/********************
	********************/
	ofSetColor(255, 255, 255, 255);
	fbo[dispVideo_id].draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case '0':
		case '1':
		case '2':
			dispVideo_id = key - '0';
			break;
			
		case 'c':
			b_test_ContentsChagne = true;
			break;
			
		case 'p':
			if(State == STATE_STOP) k_PLAY = true;
			break;
			
		case 's':
			if(State == STATE_PLAY) k_STOP= true;
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
