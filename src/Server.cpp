#include "Server.h"
#include <sstream>
#include <iostream>
#include <math.h>
#include "Timer.h"

extern Timer gTimer;

bool Server::Bind(int port){
    netBuffer.Resize(netBufferSize);

    tcp_socket = socket(AF_INET,SOCK_STREAM,0);
    if(tcp_socket < 0){
        perror("fail to create tcp socket\n");
        return false;
    }

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(tcp_socket,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        perror("fail to bind tcp socket to port\n");
        return false;
    }

    if(listen(tcp_socket,1) == -1){
        perror("fail to listen to port\n");
        return false;
    }

    for(int i = 0;i != MAX_CLINET_NUM;i++){
        connectedFd[i] = -1;
    }
    connectedFdNum = 0;

    FD_ZERO(&connectedFdSet);
    FD_SET(tcp_socket,&connectedFdSet);

    maxConnectedFD = tcp_socket;

    return true;
}

void Server::MainLoop(){
    while(1){
        /*int fd = accept(tcp_socket,NULL,NULL);
        read(fd,netBuffer.GetPtr(),netBuffer.GetSize());
    
        PROTOCOL_PARSER_STATE state = PROTOCOL_PARSER_STATE_CONTINUE;
        while(state == PROTOCOL_PARSER_STATE_CONTINUE){
            state = ProtocolParser::Buffer2CommandList(&mPost,&netBuffer);
            ParsePost();
            if(state == PROTOCOL_PARSER_STATE_FAIL) break;
        }

        close(fd);*/
        gTimer.Update();
        fd_set tmp = connectedFdSet;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;
        int readyfd = select(maxConnectedFD + 1,
            &tmp,NULL,NULL,&timeout);
        if(readyfd == 0){
            UpdateWorld();
            continue;
        }
        if(FD_ISSET(tcp_socket,&tmp)){
            int fd = accept(tcp_socket,NULL,NULL);

            if(connectedFdNum < MAX_CLINET_NUM){
                for(int i = 0;i != MAX_CLINET_NUM;i++){
                    if(connectedFd[i] < 0){
                      
                        connectedFd[i] = fd;
                        std::cout << i << " is connected" << std::endl;
                        break;
                    }
                }
                connectedFdNum++;
                maxConnectedFD = maxConnectedFD > fd ? maxConnectedFD : fd;
                FD_SET(fd,&connectedFdSet);
            }else{
                close(fd);
            }
            readyfd--;
        }
        if(readyfd != 0){
            for(int i = 0;i != MAX_CLINET_NUM;i++){
                if(connectedFd[i] < 0) continue;
                if(FD_ISSET(connectedFd[i],&tmp)){
                    PROTOCOL_PARSER_STATE state = PROTOCOL_PARSER_STATE_CONTINUE;
                    while(state == PROTOCOL_PARSER_STATE_CONTINUE){
                        int rv = read(connectedFd[i],netBuffer.GetPtr(),netBuffer.GetSize());
                        std::cout << rv << std::endl;
                        state = ProtocolParser::Buffer2CommandList(&mPost,&netBuffer);
                        ParsePost(i);
                        if(state == PROTOCOL_PARSER_STATE_FAIL) break;
                    }
                    if(mPost.head == PROTOCOL_HEAD_CLINET_DISCONNECT){
                        close(connectedFd[i]);
                        FD_CLR(connectedFd[i],&connectedFdSet);
                        std::cout << i << " is disconnected\n" << std::endl;
                        connectedFd[i] = -1;
                        connectedFdNum--;
                        continue;
                    }
                    state = PROTOCOL_PARSER_STATE_CONTINUE;
                    size_t index = 0;
                    while(state == PROTOCOL_PARSER_STATE_CONTINUE){
                        state = ProtocolParser::CommandList2Buffer(&netBuffer,&mPost,index);
                        write(connectedFd[i],netBuffer.GetPtr(),netBuffer.GetSize());
                    }
                    readyfd--;
                    if(readyfd == 0) break;
                }
            }
        }
    
    }
}

void Server::Close(){
    close(tcp_socket);
    netBuffer.Clear();
}

std::string getProtocolCommandTypeStr(PROTOCOL_COMMAND_TYPE type){
    switch(type){
        case PROTOCOL_COMMAND_TYPE_HELLO:
            return std::string("hello ");
    }
    return std::string();
}

std::string getProtocolCommandHeaderStr(PROTOCOL_HEAD type){
    switch(type){
        case PROTOCOL_HEAD_CLINET_CONNECT:
            return std::string("connect");
        case PROTOCOL_HEAD_CLINET_DISCONNECT:
            return std::string("disconnect");
        case PROTOCOL_HEAD_CLINET_MESSAGE:
            return std::string("message");
    }
    return std::string();
}

void splitstr(std::vector<std::string>& res,std::string& str,char dim){
    std::istringstream iss(str);
	std::string temp;

	res.clear();
	while (std::getline(iss,temp,dim)) {
		res.push_back(temp);
	}
}

void Server::ExcuteCommandPlayerPosition(size_t index,std::string& command){
    std::vector<std::string> data;
    splitstr(data,command,',');

    players[index].Position[0] = (float)std::stoi(data[0]) / 10000.;
    players[index].Position[1] = (float)std::stoi(data[1]) / 10000.;
    players[index].Position[2] = (float)std::stoi(data[2]) / 10000.;
    players[index].Rotation[0] = (float)std::stoi(data[3]) / 10000.;
    players[index].Rotation[1] = (float)std::stoi(data[4]) / 10000.;
    players[index].Rotation[2] = (float)std::stoi(data[5]) / 10000.;
}

std::string Server::ParsePlayerPositionCommand(size_t index){
    std::string cmd = std::to_string((int)(players[index].Position[0] * 10000.)) + ",";
    cmd += std::to_string((int)(players[index].Position[1] * 10000.)) + ",";
    cmd += std::to_string((int)(players[index].Position[2] * 10000.)) + ",";
    cmd += std::to_string((int)(players[index].Rotation[0] * 10000.)) + ",";
    cmd += std::to_string((int)(players[index].Rotation[1] * 10000.)) + ",";
    cmd += std::to_string((int)(players[index].Rotation[2] * 10000.));

    return std::move(cmd);
}

void Server::ExcuteCommandShoot(size_t index,std::string& command){
    std::vector<std::string> data;
    splitstr(data,command,',');

    Bullet bullet;
    bullet.Position[0] = (float)std::stoi(data[0]) / 10000.;
    bullet.Position[1] = (float)std::stoi(data[1]) / 10000.;
    bullet.Position[2] = (float)std::stoi(data[2]) / 10000.;
    bullet.Speed[0] = (float)std::stoi(data[3]) / 10000. * bulletSpeed;
    bullet.Speed[1] = (float)std::stoi(data[4]) / 10000. * bulletSpeed;
    bullet.Speed[2] = (float)std::stoi(data[5]) / 10000. * bulletSpeed;
    bullet.id = index;

    bullets.push_back(bullet);
}

std::string Server::ParseBulletPositionCommand(size_t index){
    std::string cmd = std::to_string((int)(bullets[index].Position[0] * 10000.)) + ",";
    cmd += std::to_string((int)(bullets[index].Position[1] * 10000.)) + ",";
    cmd += std::to_string((int)(bullets[index].Position[2] * 10000.));

    return cmd;
}

void Server::ParsePost(size_t index){

    std::cout << gTimer.Totaltime() << "s post from " << index << getProtocolCommandHeaderStr(mPost.head) << std::endl;
    for(size_t i =0;i != mPost.protocolCommands.size();i++){
        std::string& command = mPost.protocolCommands[i].command;
        std::cout << std::to_string(index) << getProtocolCommandTypeStr(mPost.protocolCommands[i].type) << command << std::endl;
    }
    if(mPost.head == PROTOCOL_HEAD_CLINET_DISCONNECT){
        return;
    }

    //parse the last post and prepare for the next post
    for(int i = 0;i != mPost.protocolCommands.size();i++){
        switch(mPost.protocolCommands[i].type){
            case PROTOCOL_COMMAND_TYPE_PLAYER_POSITION:
                ExcuteCommandPlayerPosition(index,mPost.protocolCommands[i].command);
                break;
            case PROTOCOL_COMMAND_TYPE_SHOOT:
                ExcuteCommandShoot(index,mPost.protocolCommands[i].command);
                break;
        }
    }

    mPost.protocolCommands.clear();

    mPost.head = PROTOCOL_HEAD_CLINET_MESSAGE;
    for(size_t i = 0;i != connectedFdNum;i++){
        if(i != index && connectedFd[i] >= 0){
            ProtocolCommand cmd;
            cmd.command = ParsePlayerPositionCommand(i);
            cmd.type = PROTOCOL_COMMAND_TYPE_PLAYER_POSITION;
            mPost.protocolCommands.push_back(cmd);
        }
    }

    for(size_t i  = 0;i != bullets.size();i++){
        ProtocolCommand cmd;
        cmd.command = ParseBulletPositionCommand(i);
        cmd.type = PROTOCOL_COMMAND_TYPE_BULLET_POSITION;
        mPost.protocolCommands.push_back(cmd);
    }

    size_t i = 0;
    size_t count = 0;
    while(i != events.size()){
        if(events[i].shooted != index){
            i++;
            continue;
        }
        count++;
        RemoveShoot(i);
    }

    if(count != 0){
        ProtocolCommand cmd;
        cmd.type = PROTOCOL_COMMAND_TYPE_SHOOTED;
        cmd.command = std::to_string(count);
        mPost.protocolCommands.push_back(cmd);
    }
    /*for(size_t i = 0;i != 1000;i++){
        mPost.protocolCommands.push_back({PROTOCOL_COMMAND_TYPE_HELLO,"sadadoasddkaodkoaskdo"});
    }*/


    std::cout << "post to " << index << getProtocolCommandHeaderStr(mPost.head) << std::endl;
    for(size_t i =0;i != mPost.protocolCommands.size();i++){
        std::string& command = mPost.protocolCommands[i].command;
        std::cout << std::to_string(index) << getProtocolCommandTypeStr(mPost.protocolCommands[i].type) << command << std::endl;
    }
}

float distance(float vec1[3],float vec2[3]){
    float dx = vec1[0] - vec2[0],dy = vec1[1] - vec2[1],dz = vec1[2] - vec2[2];
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void Server::UpdateWorld(){
    /*std::cout << gTimer.Totaltime() << "time:" << std::endl;
    for(int i = 0;i != MAX_CLINET_NUM;i++){
        if(connectedFd[i] > 0){
            std::cout <<"Player" << i << "state:(" << players[i].Position[0] << "," << players[i].Position[1] << "," << players[i].Position[2] << ")," <<
            "(" << players[i].Rotation[0] << "," << players[i].Rotation[1] << "," << players[i].Rotation[2] << ")" << std::endl;
        }
    }*/
    float deltatime = gTimer.Deltatime();
    int i = 0;
    while(i < bullets.size()){
        bullets[i].Position[0] += deltatime * bullets[i].Speed[0];
        bullets[i].Position[1] += deltatime * bullets[i].Speed[1];
        bullets[i].Position[2] += deltatime * bullets[i].Speed[2];

        if(abs(bullets[i].Position[0]) > 200.f || bullets[i].Position[1] > 200.f || bullets[i].Position[1] < 0.f || abs(bullets[i].Position[2]) > 200.f){
            RemoveBullet(i);
        }else{
            int hited_index = -1;
            for(size_t j = 0;j != MAX_CLINET_NUM;j++){
                if(connectedFd[j] > 0 && j != bullets[i].id){
                    float dis = distance(bullets[i].Position,players[j].Position);
                    if(dis < this->playBoundRadius){
                        hited_index = j;
                        break;
                    }
                }
            }
            if(hited_index < 0)
                i++;
            else{
                RemoveBullet(i);
                events.push_back({hited_index});
            }
        }
    }
    //std::cout << "bullet num:" << bullets.size() << std::endl;
}


void Server::RemoveBullet(size_t index){
    std::swap(bullets[index],bullets[bullets.size() - 1]);
    bullets.pop_back();
}

void Server::RemoveShoot(size_t index){
    std::swap(events[index],events[events.size() - 1]);
    events.pop_back();
}