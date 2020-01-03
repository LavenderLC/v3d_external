﻿#include "V3dR_Communicator.h"
#include "../terafly/src/control/CPlugin.h"
#include "../terafly/src/presentation/PMain.h"
#include <QRegExp>
//#include <QMessageBox>
#include <QtGui>
#include <QListWidgetItem>
#include <iostream>
#include <sstream>

FileSocket_send::FileSocket_send(QString ip,QString port,QString anofile_path,QObject *parent)
    :QTcpSocket (parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(readMSG()));
    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
    this->connectToHost(ip,port.toInt());
    QRegExp pathRex("(.*).ano");
    if(pathRex.indexIn(anofile_path)!=-1)
    {
        anopath=pathRex.cap(1);

        QFileInfo  anofile_info(anopath+".ano");
        anoname=anofile_info.fileName();
        QRegExp anoRex("(.*).ano");
        if(anoRex.indexIn(anoname)!=-1)
            anoname=anoRex.cap(1);
    }
    sendFile(anopath+".ano",anoname+".ano");
}
void FileSocket_send::sendFile(QString filepath, QString filename)
{
    QFile f(filepath);
    f.open(QIODevice::ReadOnly);
    QByteArray data=f.readAll();
    QByteArray block;

    QDataStream dts(&block,QIODevice::WriteOnly);
    dts.setVersion(QDataStream::Qt_4_7);

    dts<<qint64(0)<<qint64(0)<<filename;
    dts.device()->seek(0);
    dts<<(qint64)(block.size()+f.size());
    dts<<(qint64)(block.size()-sizeof(qint64)*2);
    dts<<filename;
    dts<<data;

    this->write(block);
    qDebug()<<filepath<<"++";
}
void FileSocket_send::readMSG()
{
    while (this->canReadLine()) {

        QRegExp anoRex("received (.*).ano\n");
        QRegExp swcRex("received (.*).eswc\n");
        QRegExp apoRex("received (.*).apo\n");
        QString MSG=QString::fromUtf8(this->readLine());
//        qDebug()<<"fileMSG:"<<MSG;
        if(anoRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.eswc",anoname+".ano.eswc");
        }else if(swcRex.indexIn(MSG)!=-1)
        {
            sendFile(anopath+".ano.apo",anoname+".ano.apo");
        }else if(apoRex.indexIn(MSG)!=-1)
        {
//            qDebug()<<"file upload is ok.";
            this->disconnectFromHost();
            QMessageBox::information(0, tr("information"),tr("import successfully."));
        }
    }
}
ManageSocket::ManageSocket(QObject *parent):QTcpSocket (parent)
{
    MSGsocket=0;FileRec=0;
    messageport.clear();
    loadfilename.clear();
//    connect(this,SIGNAL(disconnected()),this,SLOT(deleteLater()));
}
void ManageSocket::onReadyRead()
{
    QRegExp LoginRex("(.*):log in success.\n");
    QRegExp LogoutRex("(.*):log out success.\n");
    QRegExp ImportRex("(.*):import port.\n");
    QRegExp CurrentDirDownExp("(.*):currentDir_down.\n");
    QRegExp CurrentDirLoadExp("(.*):currentDir_load.\n");
    QRegExp MessagePortExp("(.*):messageport.\n");

	if(this->canReadLine())
	{
		QString manageMsg=QString::fromUtf8(this->readLine());
       
		if(LoginRex.indexIn(manageMsg)!=-1)
		{
			QMessageBox::information(0, tr("information"),tr("login successfully."));
		}else if (LogoutRex.indexIn(manageMsg)!=-1)
		{
			this->disconnectFromHost();
		}else if(ImportRex.indexIn(manageMsg)!=-1)
		{
            if(this->state()==QAbstractSocket::UnconnectedState)
            {
                QMessageBox::information(0, tr("Error"),tr("can not connect with Manageserver."));
                return ;
            }

            QString anofile_path = QFileDialog::getOpenFileName(0,"标题",".","*.ano");
            if(anofile_path.isEmpty()) return;
            FileSocket_send *filesocket_send=new FileSocket_send(ip,ImportRex.cap(1),anofile_path);

        }else if(CurrentDirDownExp.indexIn(manageMsg)!=-1)
		{
            QString currentDir=CurrentDirDownExp.cap(1);
			QStringList file_list=currentDir.split(";");
			QWidget *widget=new QWidget(0);
			widget->setWindowTitle("choose annotation file ");
			QListWidget *filelistWidget=new QListWidget;
			QVBoxLayout mainlayout(widget);

			mainlayout.addWidget(filelistWidget);
			connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,SLOT(send1(QListWidgetItem*)));

			connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
				widget,SLOT(close()));

			filelistWidget->clear();
			for(uint i=0;i<file_list.size();i++)
			{
				QIcon icon("file.png");
				QListWidgetItem *tmp=new QListWidgetItem(icon,file_list.at(i));
				qDebug()<<file_list.at(i);
				filelistWidget->addItem(tmp);
			}
			widget->show();
        }else if(CurrentDirLoadExp.indexIn(manageMsg)!=-1)
        {
            QString currentDir=CurrentDirLoadExp.cap(1);
            QStringList file_list=currentDir.split(";");
            QListWidget *filelistWidget=new QListWidget;
            filelistWidget->setWindowTitle("choose annotation file ");
            filelistWidget->setGeometry(400,400,500,700);
            connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                this,SLOT(send2(QListWidgetItem*)));
            connect(filelistWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                filelistWidget,SLOT(close()));
            filelistWidget->clear();
            for(uint i=0;i<file_list.size();i++)
            {
                QListWidgetItem *tmp=new QListWidgetItem(file_list.at(i));
                filelistWidget->addItem(tmp);
            }
            filelistWidget->show();
        }else if(MessagePortExp.indexIn(manageMsg)!=-1)
        {
            messageport=MessagePortExp.cap(1);
            emit makeMessageSocket(ip,MessagePortExp.cap(1),name);
        }

	}
}
void ManageSocket::send1(QListWidgetItem *item)
{
    FileSocket_receive *filesocket_receive=new FileSocket_receive(ip);
    disconnect(filesocket_receive,SIGNAL(receivefile(QString)),this,SLOT(receivefile(QString)));
    this->write(QString(item->text()+":choose1."+"\n").toUtf8());
}
void ManageSocket::send2(QListWidgetItem *item)
{
	loadfilename.clear();FileRec=0;
    FileSocket_receive *filesocket_receive=new FileSocket_receive(ip);
    connect(filesocket_receive,SIGNAL(receivefile(QString)),this,SLOT(receivefile(QString)));
    loadfilename=item->text();
    this->write(QString(item->text()+":choose2."+"\n").toUtf8());
}
void ManageSocket::messageMade()
{
    MSGsocket=1;

    if(FileRec==1)
        emit loadANO(loadfilename);
}
void ManageSocket::receivefile(QString anofile)
{
    FileRec=1;

    if(MSGsocket==1)
        emit loadANO(loadfilename);
}
V3dR_Communicator::V3dR_Communicator(bool *client_flag /*= 0*/, V_NeuronSWC_list* ntlist/*=0*/)
{
	clienton = client_flag;
//	NTList_SendPool = ntlist;
	NTNumReceieved=0;
	NeuronTree  nt = terafly::PluginInterface::getSWC();
	int tempntsize = nt.listNeuron.size();
	CreatorMarkerPos = 0;
	CreatorMarkerRes = 0;
	userName="";
	QRegExp regex("^[a-zA-Z]\\w+");
    socket = 0;
	CURRENT_DATA_IS_SENT=false;
    asktimer=nullptr;
    nextblocksize=0;
    AutoTraceNode=XYZ(0);
    flag_x=0;flag_y=0;flag_z=0;
    if(asktimer==nullptr)
    {
        asktimer=new QTimer(this);
        connect(asktimer,SIGNAL(timeout()),this,SLOT(askserver()));
//                asktimer->start(1000);
    }
}
V3dR_Communicator::~V3dR_Communicator() {
}

void V3dR_Communicator::timerStart(QString anoname,int mesc)
{
    //读取，使用TP处理
    qDebug()<<"adsas";
    QFile *f=new QFile("./clouddata/"+anoname+".txt");

    if(f->open(QIODevice::ReadOnly|QIODevice::Text))
    {
        while (!f->atEnd()) {
            QByteArray line=f->readLine();
            TFProcess(QString(line),1);
        }
    }

    delete  f;

    asktimer->start(mesc);
}
bool V3dR_Communicator::SendLoginRequest(QString ip,QString port,QString user) {

    socket=new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(this,SIGNAL(msgtoprocess(QString)),this,SLOT(TFProcess(QString)));
    connect(socket,SIGNAL(connected()),this,SLOT(onConnected()));

//    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    QSettings settings("HHMI", "Vaa3D");

    userName=user;
//    vr_Port=port;
    Agent agent00={
        //with local information
        userName,
        true,//means this struct point to itself,no need to render
        21,
        0,
    };
    Agents.push_back(agent00);
	
    socket->connectToHost(ip, port.toUInt());
	if(!socket->waitForConnected(15000))
	{
		if(socket->state()==QAbstractSocket::UnconnectedState)
		{
			qDebug()<<"Cannot connect with Server. Unknown error.";
			return 0;
		}	
	}
    qDebug()<<"User:  "<<userName<<".  Connected with server: "<<ip<<" :"<<port;
    emit messageMade();

	return 1;
}
void V3dR_Communicator::UpdateSendPoolNTList(V_NeuronSWC seg)
{
//	NTList_SendPool->append(seg);

    onReadySend(QString("/seg: "+V_NeuronSWCToSendMSG(seg)));
    cout << "send seg success" << endl;
}

void V3dR_Communicator::UpdateDeleteMsg(vector<XYZ> deleteLocNode)
{
	onReadySend(QString("/del_curve: " + V_DeleteNodeToSendMSG(deleteLocNode)));
	cout << "send delete over success" << endl;
}

void V3dR_Communicator::UpdateSendPoolNode(float X, float Y, float Z,int type)
{
    XYZ global_node=ConvertLocaltoGlobalCroods(X,Y,Z);
//    qDebug()<<"global_node"<<global_node.x<<" "<<global_node.y<<" "<<global_node.z;
    QString nodeMSG=QString("/marker:"+QString::number(global_node.x)+" "
                            +QString::number(global_node.y)+" "+QString::number(global_node.z)
                            +" "+QString::number(ImageCurRes.x)+" "+QString::number(ImageCurRes.y)
                            +" "+QString::number(ImageCurRes.z)+" "+QString::number(type));
    AutoTraceNode=XYZ(global_node.x,global_node.y,global_node.z);
    onReadySend(nodeMSG);

}

void V3dR_Communicator::UpdateSendDelMarkerInfo(float x,float y,float z)
{
    XYZ global_node=ConvertLocaltoGlobalCroods(x,y,z);
    QString nodeMSG=QString("/marker:"+QString::number(global_node.x)+" "
                            +QString::number(global_node.y)+" "+QString::number(global_node.z)
                            +" "+QString::number(ImageCurRes.x)+" "+QString::number(ImageCurRes.y)
                            +" "+QString::number(ImageCurRes.z));
    onReadySend(nodeMSG);
}
void V3dR_Communicator::askserver()
{
    onReadySend(QString("/ask:message"));
}


void V3dR_Communicator::onReadySend(QString send_MSG) {

    if (!send_MSG.isEmpty()) {
        if((send_MSG!="exit")&&(send_MSG!="quit"))
        {
        }
        else
        {
            send_MSG="/say: GoodBye~";
        }
        QByteArray block;
        QDataStream dts(&block,QIODevice::WriteOnly);
        dts.setVersion(QDataStream::Qt_4_7);

        dts<<quint16(0)<<send_MSG;
        dts.device()->seek(0);
        dts<<quint16(block.size()-sizeof (quint16));
        socket->write(block);
        socket->flush();
	}
	else
	{
//		qDebug()<<"The message is empty!";
        return;
	}
}

void V3dR_Communicator::onReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_7);
    QString line;
//    int i=0;

    while(1)
    {
        if(nextblocksize==0)
        {
            if(socket->bytesAvailable()>=sizeof (quint16))
            {
                in>>nextblocksize;
            }
            else
            {
//                qDebug()<<"bytes <quint16";
                return;
            }
        }

        if(nextblocksize>0&&socket->bytesAvailable()>=nextblocksize)
        {
            in >>line;
//            qDebug()<<line;
//            i++;
            nextblocksize=0;
//            QStringList iniaial_list=line.split(",");

//            int cnt=iniaial_list.size();
//            if(cnt>1)
//            {
//                for(int i=0;i<cnt;i++)
//                {

//                    TFProcess(iniaial_list.at(i),1);
//                }
//            }
//            else
                emit msgtoprocess(line);
        }else
        {
            return ;
        }

    }
}


void V3dR_Communicator::TFProcess(QString line,bool flag_init) {
    QRegExp usersRex("^/users:(.*)$");
    QRegExp systemRex("^/system:(.*)$");
    QRegExp hmdposRex("^/hmdpos:(.*)$");
    QRegExp colorRex("^/color:(.*)$");
    QRegExp deletecurveRex("^/del_curve:(.*)__(.*)$");
    QRegExp markerRex("^/marker:(.*)__(.*)$");
    QRegExp delmarkerRex("^/del_marker:(.*)__(.*)$");
    QRegExp dragnodeRex("^/drag_node:(.*)$");
    QRegExp creatorRex("^/creator:(.*)$");
    QRegExp messageRex("^/seg:(.*)__(.*)$");


        line=line.trimmed();
        if (usersRex.indexIn(line) != -1) {
            QStringList users = usersRex.cap(1).split(",");
            foreach (QString user, users) {
                if(user==userName) continue;// skip itself
                bool findSameAgent=false;
                for(int i=0;i<Agents.size();i++)
                {
                    if(user==Agents.at(i).name)
                    {
                        findSameAgent=true;
                        break;
                    }
                }
                if(findSameAgent==false)
                {
                    Agent agent00={
                        user,
                        false,
                        21,
                        0

                    };
                    Agents.push_back(agent00);
                }
            }

        }
        else if (systemRex.indexIn(line) != -1) {

            QStringList sysMSGs = systemRex.cap(1).split(" ");
            if(sysMSGs.size()<2) return;
            QString user=sysMSGs.at(0);
            QString Action=sysMSGs.at(1);
            if((user!=userName)&&(Action=="joined"))
            {
                Agent agent00={
                    user,
                    false,
                    21,//colortypr
                    0, //POS
                };
                Agents.push_back(agent00);
            }
            else if((user!=userName)&&(Action=="left"))
            {
                qDebug()<<"user: "<< user<<"left";
                for(int i=0;i<Agents.size();i++)
                {
                    if(user == Agents.at(i).name)
                    {
                        Agents.erase(Agents.begin()+i);
                        i--;
                    }
                }
            }

        }
        else if(colorRex.indexIn(line) != -1) {
//            qDebug()<<"incolor";
            QStringList clrMSGs = colorRex.cap(1).split(" ");

            if(clrMSGs.size()<2) return;
            QString user=clrMSGs.at(0);
            QString clrtype=clrMSGs.at(1);
            for(int i=0;i<this->Agents.size();i++)
            {
                if(this->Agents.at(i).name!=user) continue;
                this->Agents.at(i).colorType=clrtype.toInt();
//                qDebug()<<"user:"<<user<<" receievedColorTYPE="<<this->Agents.at(i).colorType;
            }
        }
        else if (deletecurveRex.indexIn(line) != -1) {
            QString user=deletecurveRex.cap(1);
            qDebug()<<"+============delseg process begin========";
            if(flag_init==0)
            {
                if(user!=userName)
                    emit delSeg(deletecurveRex.cap(2).trimmed());
                else
                    qDebug()<<"user:"<<user<<"==userName"<<userName;
            }else {
                emit delSeg(deletecurveRex.cap(2).trimmed());
            }
            qDebug()<<"+============delseg process end========";

        }
        else if (markerRex.indexIn(line) != -1) {

            qDebug()<<"+============marker process begin========";
            QString user=markerRex.cap(1);

            int colortype=21;
            for(int i=0;i<Agents.size();i++)
            {
                if(Agents[i].name==user)
                {
                    colortype=Agents[i].colorType;
                    qDebug()<<"marker color"<<colortype;
                    break;
                }
            }
            if(flag_init==0)
            {
                if(user!=userName)
                {
                    emit addMarker(markerRex.cap(2).trimmed(),3);//marker用3号色
                    qDebug()<<"siagnal add marker";
                }
                else
                    qDebug()<<"user:"<<user<<"==userName"<<userName;
            }else {
                emit addMarker(markerRex.cap(2).trimmed(),colortype);//marker用3号色
            }

            qDebug()<<"==================marker process end====================";

        }
        else if (delmarkerRex.indexIn(line) != -1) {

        }
        else if (messageRex.indexIn(line) != -1) {

            qDebug()<<"======================messageRex in Terafly begin============";

            QString user=messageRex.cap(1);
            int colortype=21;
            for(int i=0;i<Agents.size();i++)
            {
                if(Agents[i].name==user)
                {
                    colortype=Agents[i].colorType;break;
                }
            }
            QString temp1=messageRex.cap(2).trimmed();

            QString temp=temp1.split("_").at(0).trimmed().split(" ").at(0);
            if(flag_init==0)
            {
                if(user==userName&&temp=="TeraFly")
                    qDebug()<<"user:"<<user<<"==userName"<<userName;
                else
                {
                    emit addSeg(temp1,colortype);
                }
            }else {
                emit addSeg(temp1,colortype);
            }

        }

        else if(creatorRex.indexIn(line)!=-1)
        {
            qDebug()<<line;
            QString user = creatorRex.cap(1);
            QStringList markerMSGs = creatorRex.cap(2).split(" ");
            if (markerMSGs.size() < 3)
            {
                qDebug() << "size < 3";
                return;
            }

            float mx = markerMSGs.at(0).toFloat();
            float my = markerMSGs.at(1).toFloat();
            float mz = markerMSGs.at(2).toFloat();
            int resx, resy, resz,res;

            if (markerMSGs.size() > 3)
            {
                resx = markerMSGs.at(3).toFloat();
                resy = markerMSGs.at(4).toFloat();
                resz = markerMSGs.at(5).toFloat();
                res = markerMSGs.at(6).toInt();

            }
            emit addMarker(markerRex.cap(2).trimmed(),3);//para2 was not used
            //wait ...

        }
}

void V3dR_Communicator::CollaborationMainloop(){
	Collaborationsendmessage();
	Collaborationaskmessage();
	QTimer::singleShot(200, this, SLOT(CollaborationMainloop()));
}
void V3dR_Communicator::onConnected() {

//    qDebug()<<"gere is onconnected.";
    onReadySend(QString("/login:" +userName));

}
void V3dR_Communicator::Collaborationsendmessage()
{
    onReadySend(QString("/say: send~"));//????
}


void V3dR_Communicator::Collaborationaskmessage()
{
        onReadySend(QString("/ask: message"));
}

QString V3dR_Communicator::V_NeuronSWCToSendMSG(V_NeuronSWC seg)
{
    QString messageBuff=QString("TeraFly %1 %2 %3_").arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);

	for(int i=0;i<seg.row.size();i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
	{

		V_NeuronSWC_unit curSWCunit = seg.row[i];
        QString packetbuff;
        packetbuff.clear();
        if(i!=seg.row.size()-1)
        {
            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z);
            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11_").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
                    .arg(curSWCunit.r).arg(curSWCunit.parent).arg(curSWCunit.level).arg(curSWCunit.creatmode).arg(curSWCunit.timestamp).arg(curSWCunit.tfresindex);

            if(i==0)
                 emit delMarker(QString("%1 %2 %3").arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z));
        }
        else
        {
            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z);
            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
                    .arg(curSWCunit.r).arg(curSWCunit.parent).arg(curSWCunit.level).arg(curSWCunit.creatmode).arg(curSWCunit.timestamp).arg(curSWCunit.tfresindex);
            emit addMarker(QString("%1 %2 %3").arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z),curSWCunit.type);
            AutoTraceNode=XYZ(GlobalCroods.x,GlobalCroods.y,GlobalCroods.z);
        }
        messageBuff +=packetbuff;
	}

    if(seg.row.size()>5)
    {
        if(seg.row[5].x-seg.row[0].x>0) flag_x=1;else flag_x=-1;
        if(seg.row[5].y-seg.row[0].y>0) flag_y=1;else flag_y=-1;
        if(seg.row[5].z-seg.row[0].z>0) flag_z=1;else flag_z=-1;
//        qDebug()<<"flag(x,y,z)="<<flag_x<<" "<<flag_y<<" "<<flag_z;
    }
    return messageBuff;
}

QString V3dR_Communicator::V_NeuronSWCToSendMSG(V_NeuronSWC seg,XYZ* para)
{
//    char extramsg[300];
    QString messageBuff=QString("TeraAI %1 %2 %3_").arg(ImageCurRes.x).arg(ImageCurRes.y).arg(ImageCurRes.z);

    for(int i=0;i<seg.row.size();i++)   //why  i need  < 120, does msg has length limitation? liqi 2019/10/7
    {
        V_NeuronSWC_unit curSWCunit = seg.row[i];
        QString packetbuff;
        packetbuff.clear();
        if(i!=seg.row.size()-1)
        {
            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z,para);

            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7_").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
                    .arg(curSWCunit.r).arg(curSWCunit.parent);

        }
        else
        {
            XYZ GlobalCroods = ConvertLocaltoGlobalCroods(curSWCunit.x,curSWCunit.y,curSWCunit.z,para);
            packetbuff=QString("%1 %2 %3 %4 %5 %6 %7").arg(curSWCunit.n).arg(curSWCunit.type).arg(GlobalCroods.x).arg(GlobalCroods.y).arg(GlobalCroods.z)
                    .arg(curSWCunit.r).arg(curSWCunit.parent);
        }
        messageBuff+=packetbuff;
    }
    return messageBuff;
}



QString V3dR_Communicator::V_DeleteNodeToSendMSG(vector<XYZ> loc_list)
{
    string messageBuff ;
	for (int i = 0; i < loc_list.size(); ++i)
	{
		char packetbuff[300];
		XYZ GlobalCroods = ConvertLocaltoGlobalCroods(loc_list[i].x, loc_list[i].y, loc_list[i].z);
        sprintf(packetbuff, "%5.3f %5.3f %5.3f %5.3f %5.3f %5.3f_", GlobalCroods.x, GlobalCroods.y, GlobalCroods.z, ImageCurRes.x, ImageCurRes.y, ImageCurRes.z);

		messageBuff += packetbuff;
	}
	QString out_MSG = QString::fromStdString(messageBuff);
	return out_MSG;
}

void V3dR_Communicator::MsgToV_NeuronSWC(QString msg)
{
	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
	int str_size = qsl.size()-(qsl.size()%3);//to make sure that the string list size always be 3*N;
	vector<XYZ> loclist_temp;
	XYZ loc_temp;
	for(int i=0;i<str_size;i++)
	{
		qsl[i].truncate(99);
		int iy = i%7;
		if (iy==0)
		{
			loc_temp.x = qsl[i].toFloat();
		}
		else if(iy==1)
		{
			loc_temp.y = qsl[i].toFloat();
		}
		else if(iy==2)
		{
			loc_temp.z = qsl[i].toFloat();
			loclist_temp.emplace_back(loc_temp);
		}

	}
	loc_ReceivePool.push_back(loclist_temp);
	//update AddCurveSWC
}

void V3dR_Communicator::onDisconnected() {
    qDebug("Now disconnect with the server."); 
	*clienton = false;
    deleteLater();
}


XYZ V3dR_Communicator::ConvertGlobaltoLocalCroods(double x,double y,double z)
{
//	cout << "ImageCurRes" << endl;
	cout << ImageCurRes.x << endl;
	cout << ImageCurRes.y << endl;
	cout << ImageCurRes.z << endl;
	x/=(ImageMaxRes.x/ImageCurRes.x);
	y/=(ImageMaxRes.y/ImageCurRes.y);
	z/=(ImageMaxRes.z/ImageCurRes.z);
	x-=(ImageStartPoint.x-1);
	y-=(ImageStartPoint.y-1);
	z-=(ImageStartPoint.z-1);
	return XYZ(x,y,z);
}

XYZ V3dR_Communicator::ConvertLocaltoGlobalCroods(double x,double y,double z)
{

	x+=(ImageStartPoint.x-1);
	y+=(ImageStartPoint.y-1);
	z+=(ImageStartPoint.z-1);
	x*=(ImageMaxRes.x/ImageCurRes.x);
	y*=(ImageMaxRes.y/ImageCurRes.y);
	z*=(ImageMaxRes.z/ImageCurRes.z);
	return XYZ(x,y,z);
}

XYZ V3dR_Communicator::ConvertLocaltoGlobalCroods(double x,double y,double z,XYZ* para)
{
    //Para={MaxRes, start_global,start_local}
    x+=para[1].x-para[2].x;
    y+=para[1].y-para[2].y;
    z+=para[1].z-para[2].z;
    return XYZ(x,y,z);
}

void V3dR_Communicator::read_autotrace(QString path,XYZ* tempPara)
{

    NeuronTree auto_res=readSWC_file(path);
    V_NeuronSWC_list testVNL= NeuronTree__2__V_NeuronSWC_list(auto_res);

    if(testVNL.seg.size()!=0)
    {
        for(int i=0;i<testVNL.seg.size();i++)
        {

            V_NeuronSWC seg_temp =  testVNL.seg.at(i);
            qDebug()<<V_NeuronSWCToSendMSG(seg_temp,tempPara);
            onReadySend(QString("/seg: "+V_NeuronSWCToSendMSG(seg_temp,tempPara)));
        }
    }
}




