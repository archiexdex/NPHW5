#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <iostream>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <typeinfo>
#include <map>
#include <string>
#include <pthread.h>
#include <vector>
#include <time.h>
#include <semaphore.h>

#define maxcli 50
using namespace std;

bool optest(string,char);

vector<int> user;
map<int,string> usernames;
map<int,vector<string> > mails;
map<int,map<string,bool> > isread;
map<int,vector<string> > datas;

struct tm *T;
time_t t;


string gettime(struct tm *T){
	char s[1000];
	sprintf(s,"%d-%d-%d %d:%d:%d",
		T->tm_year+1900,T->tm_mon+1,T->tm_mday,T->tm_hour,T->tm_min,T->tm_sec);
	string tmp = s;
	return s;
}

string getcommand(char* S , int& st){
	string command = "";
	int i = st;
	for( ; S[i] == ' ' ; i++ );
	for( ; S[i] != ' ' ; i++ ){
		if( S[i] == '\0' ) break;
		command += S[i];
	}
	st = i;
	return command;
}

char getopts(char* S,  int st , int n){
	int i = st;
	while(n--){
		for( ; S[i] != '-' ; i++ ){
			if( S[i] == '\0' ) break;
		}
		i++;
	}
	return S[i];
}

string getargss(char* S , int st , int n){
	int i = st;
	string tmp = "";
	while(n!=0){
		
		for( ; S[i] != '-' ; i++ ){
			if( S[i] == '\0' ) break;
		}
		i+= 2;
		for( ; S[i] == ' ' ; i++ ){
			if( S[i] == '\0' ) break;
		}
		if( n > 1 )
			for( ; S[i] != ' ' ; i++ ){
					if( S[i] == '\0' ) break;
				}
		else 
			for( ; S[i] != ' ' ; i++ ){
				if( S[i] == '\0' ) break;
				tmp += S[i];
			}
		--n;
	}
//	cout << tmp << " $$$ " << endl;
	return tmp;
}

char getop(char* S , int& st){
	int i = st;
	
	string tmp = "";
	for( ; S[i] == ' ' ; i++ ){
		if( S[i] == '\0' ) return S[i];
	}
	st = i+2;
	if( S[i] != '-' ) return 'Z';
	else return S[++i];
}
string getarg(char* S , int& st){
	int i = st;cout << S << " st" << endl;
	bool good = true , quo = false;
	string tmp = "";
	for( ; S[i] == ' ' ; i++ ){
		if( S[i] == '\0' ){
			good = false;
			break;
		}
	}
	if( S[i] == '"' ) quo = true;
//	cout << quo << "TT" << endl;
	if( quo )
		for( ++i; S[i] != '"' ; i++ ){
			tmp += S[i];
		}
	else
		for( ; S[i] != ' ' ; i++ ){
			if(S[i] == '\0'){
				good = false;
				break;
			}
			tmp += S[i];
		}
	if (good)st = i+1;
	else st = i;
	return tmp;
}
string getnum(char* S , int& st , bool& isnum){
	int i = st;cout << S << " st" << endl;
	bool good = true , quo = false;
	string tmp = "";
	for( ; S[i] == ' ' ; i++ ){
		if( S[i] == '\0' ){
			good = false;
			break;
		}
	}
	if( S[i] == '"' ) quo = true , isnum = false;
	cout << quo << "TT" << endl;
	if( quo )
		for( ++i; S[i] != '"' ; i++ ){
			tmp += S[i];
		}
	else
		for( ; S[i] != ' ' ; i++ ){
			if(S[i] == '\0'){
				good = false;
				break;
			}
			if( S[i] < '0' || S[i] > '9' ) isnum = false;
			tmp += S[i];
		}
	if (good)st = i+1;
	else st = i;
	return tmp;
}
bool nametest(string);
bool rtctest(string);
string getacc(string);
string getdata(string,string);
string newre(string);
string newfwd(string);

void *solve(void *socket){
	
	char S[2048] , buf[2048] ;
	string command="" , account = "";
	char op;
	int err , rec , s , client = *(int*)socket , st = 0;
	bool isinit = false , isexit = false;
	
	while( 1 ){
		
		fflush( stdout );
		if( client < 0 )
			printf("error on accept\n");
		memset(buf,0,sizeof(buf));
		memset(S,0,sizeof(S));
		
		printf("now is %d@@\n",client);
		rec = recv(client , buf , sizeof(buf) , 0 );
		buf[rec] = '\0'; buf[rec+1] = 0;
		printf("%s!! %d XD\n",buf,rec);
		printf("---------------------------------------------\n");
//		scanf("%*s");
		st = 0;
		command = getcommand(buf,st);
		err = 0;
		cout << "command: " << command << endl;
		if( command == "exit" ){
			printf("in exiting command\n");
			usernames.erase(client);
			mails.erase(client);
			isread.erase(client);
			datas.erase(client);
			sprintf(S,"exit\n");
			isexit = true;
		}
		else if( command == "init" ){
			cout << "in init!!" << endl;
			op = getop(buf,st);
			if( optest(command,op) )
				err = 2;
			else {
				account = getarg(buf,st);
				if( nametest(account) ) err = 3;
				else {
				bool flg = true;
				for( map<int,string>::iterator it = usernames.begin() ; it!=usernames.end();++it ){
					if ( account == it->second ) {
						flg = false;
						sprintf(S,"This account has been registered\n");
						break;
					}
					
				}
				if( usernames.find(client) != usernames.end() ){
					sprintf(S,"This account has been registered\n");
					flg = false;
				}
				if( flg ){
					isinit = true;
					usernames[client] = account;
					mails[client] = vector<string>();
					isread[client] = map<string,bool>();
					datas[client] = vector<string>();
					sprintf(S,"%s@nctu.edu.tw\n",account.c_str() );
				}
				}
			}
		}
		else if( command == "ls" ){
			int o = 0;
			
			op = getop(buf,st);
			
			char other = getopts(buf,2,2);puts("!!");
			string another = getargss(buf,2,1);
			cout << another.c_str() << " # "  << other << endl; 
			if( other != '\0' || another != "\0") err = 3;
			else{
			
			if( optest(command,op) )
				err = 2;
			if( op == 'u' ){
				if( usernames.empty() ){
					sprintf(S,"no accounts\n");
				}
				puts("GG");
				string tmp = "";
				for( map<int,string>::iterator it = usernames.begin() ; it != usernames.end() ; ++it ){
					tmp += it->second;
					tmp += "@nctu.edu.tw\n";
				}
				sprintf(S,"%s",tmp.c_str());
			}
			else if( op == 'l' ){
				if( !isinit ){
					sprintf(S,"init first\n");
				}
				else{
					if( mails[client].empty() ){
						sprintf(S,"no mail\n");
					}
					else{
						string tmp = "";
						for( int i = 0 ; i < mails[client].size() ; ++i){
							tmp += (i+1) + 48; tmp += ". "; tmp += mails[client][i];
							if( !isread[client].find(mails[client][i])-> second )
								tmp += "(new)";
							tmp += "\n";
						}
						sprintf(S,"%s",tmp.c_str() );
					}
				}
			}
			else if( op == 'a' ){
				if( !isinit ){
					sprintf(S,"init first\n");
				}
				else {
					string account = usernames.find(client)->second;
					string tmp = "Account: ";
					tmp += account+"\n";
					tmp += "Mail address: ";
					tmp += account+"@nctu.edu.tw\n";
					tmp += "Number of mails: "; tmp += mails[client].size()+48;
					tmp += "\n";
					sprintf(S,"%s",tmp.c_str() );
				}
			}
			
			}
		}
		else if( command == "rm" ){
			if( !isinit ){
				sprintf(S,"init first\n");
			}
			op = getop(buf,st);
			bool isnum = true;
			if( optest(command,op) ){
				err = 2;
			}
			if( op == 'd' ){
				int n = atoi( getnum(buf,st,isnum).c_str() );
				if( !isnum || mails[client].size() < n ){
					err = 3;
				}
				else {puts("0");
					string tmp = mails[client][n-1];
					mails[client].erase(mails[client].begin()+n-1);
					isread[client].erase( isread[client].find(tmp) );
					datas[client].erase(datas[client].begin()+n-1);
					sprintf(S,"done\n");
				}
			}
			else if( op == 'D' ){
				while( !mails[client].empty() ){
					mails[client].pop_back();
					datas[client].pop_back();
				}
				isread[client].clear();
				sprintf(S,"done\n");
			}
		}
		else if( command == "rd" ){
			if( !isinit ){
				sprintf(S,"init first\n");
			}
			else{
				op = getop(buf,st);
				bool isnum = true;
				if( optest(command,op) )
					err = 2;
				else {
					int n = atoi( getnum(buf,st,isnum).c_str() );
					
					cout << datas[client].size() << " and " << n << "@@" <<endl;
					if( !isnum || mails[client].size() < n )
						err = 3;
					else {
						string goal = mails[client][n-1];
						isread[client][goal] = true;
						string tmp = datas[client][n-1];
						printf("in reading command OAO\n");
						
						sprintf(S,"%s",tmp.c_str() );
					}
				}
			}
		}
		else if( command == "wt" ){
			if( !isinit ){
				sprintf(S,"init first\n");
			}
			else {
				int o = 0;
				time(&t);
				T = localtime(&t);
				string now = gettime(T);
				string receiver = "" , title = "" , content = "";
				while ( o < 3 ){
					op = getop(buf,st);
					printf("option is %c , times is %d!  \n",op,o);
					if( optest(command,op) ){
						err = 2;
						break;
					}
					else {
						string args = getarg(buf,st);
						cout << args << " OAO " << endl;
						if( op == 'd' ){
							receiver = args;
						}
						else if( op == 't' ){
							title = args;
						}
						else if( op == 'c' ){
							content = args;
						}
					}
					o++;
				}
				string others = getarg(buf,st);
				if( others != "\0" ) err = 2 , o = 0;
				if( rtctest(receiver) && rtctest(title) && rtctest(content) ) err = 3 , o = 0;
				cout << receiver << " " << title << " " << content << endl;
				printf("in writing command!!!\n");
				if( o == 3 ) {
					account = getacc(receiver);
					int dir;
					bool isexist = false;
					for( map<int,string>::iterator it = usernames.begin(); it != usernames.end() ; ++it )
						if( it->second == account ){
							dir = it->first;isexist = true;
						}
					if( !isexist ) err = 3;
					else{
						mails[dir].push_back(title);
						isread[dir][title] = false;
						string tmp = "From: "; tmp += usernames[client] + "@nctu.edu.tw\n";
						tmp += "To: "; tmp += receiver + "\n";
						tmp += "Date: "; tmp += now + "\n";
						tmp += "Title: "; tmp += title + "\n";
						tmp += "Content: "; tmp += content + "\n";
						datas[dir].push_back(tmp);
						
						sprintf(S,"done\n");
					}
				}
			}
		}
		else if( command == "re" ){
			if( !isinit ){
				sprintf(S,"init first\n");
			}
			else {
				int o = 0 , n;
				time(&t);
				T = localtime(&t);
				string now = gettime(T);
				string content = "";
				while( o < 2 ){
					op = getop(buf,st);
					printf("option is %c , times is %d!  \n",op,o);
					if( optest(command,op) ){
						err = 2;
						break;
					}
					else{
						if( op == 'c' ){
							string arg = getarg(buf,st);
							cout << arg << " PPP " << endl;
							content = arg;
						}
						else if( op == 'n' ){
							bool isnum = true;
							n = atoi( getnum(buf,st,isnum).c_str() );
							if( !isnum ){
								err = 2;break;
							}
						}
					}
				++o;
				}
				cout << content << " " << n << endl;
				if( o == 2 ){
					string tmp = "" , data = datas[client][n-1];
					string from = getdata(data,"From: ") , to = getdata(data,"To: ");
					string title = getdata(data,"Title: ");
					title = newre(title);
					tmp += "From: "; tmp += to+ "\n";
					tmp += "To: "; tmp += from+ "\n";
					tmp += "Date: " ; tmp += now + "\n";
					tmp += "Title: " ; tmp += title + "\n";
					tmp += "Content: " ; tmp += content += "\n";
					tmp += "----\n"; tmp += data;
					string account = getacc(from);
					int dir;
					for( map<int,string>::iterator it=usernames.begin(); it!=usernames.end();++it ){
						if( account == it->second ){
							dir = it->first;break;
						}
					}
					
					mails[dir].push_back(title);
					isread[dir][title] = false;
					datas[dir].push_back(tmp);
					sprintf(S,"done\n");
				}
			}
		}
		else if( command == "fwd" ){
			if( !isinit ){
				sprintf(S,"init first\n");
			}
			else {
				int o = 0 , n;
				time(&t);
				T = localtime(&t);
				string now = gettime(T);
				string content = "" , receiver = "";
				while( o < 3 ){
					op = getop(buf,st);
					printf("option is %c , times is %d!  \n",op,o);
					if( optest(command,op) ){
						err = 2;
						break;
					}
					else{
						if( op == 'd' ){
							string arg = getarg(buf,st);
							cout << arg << " FFF " << endl;
							receiver = arg;
						}
						else if( op == 'n' ){
							bool isnum = true;
							n = atoi( getnum(buf,st,isnum).c_str() );
							cout << n << " NNN " << endl;
							if( !isnum ){
								err = 2;break;
							}
						}
						else if( op == 'c' ){
							string arg = getarg(buf,st);
							cout << arg << " VVV " << endl;
							content = arg;
						}
					}
				++o;
				}
				cout << content << " " << receiver << " " << n << endl;
				printf("in forwarding!!\n");
				if( o == 3 ){
					string account = getacc(receiver);
					int dir;
					bool isexist = false;
					for( map<int,string>::iterator it = usernames.begin(); it != usernames.end() ; ++it )
						if( it->second == account ){
							dir = it->first;isexist = true;break;
						}
					if( !isexist ) err = 3;
					else{
						string tmp = "" , data = datas[client][n-1];
						string title = getdata(data,"Title: ");cout << title << " RRR " << endl;
						title =  newfwd(title);
						cout << title << " is title\n";
						mails[dir].push_back(title);
						isread[dir][title] = false;
						tmp = "From: "; tmp += usernames[client] + "@nctu.edu.tw\n";
						tmp += "To: "; tmp += receiver + "\n";
						tmp += "Date: "; tmp += now + "\n";
						tmp += "Title: "; tmp += title + "\n";
						tmp += "Content: "; tmp += content + "\n";
						tmp += "----\n"; tmp += data;
						datas[dir].push_back(tmp);
						
						sprintf(S,"done\n");
					}
				}
			}
		}
		else {
			err = 1;
		}
		
		switch(err){
			case 1:
				sprintf(S,"command error\n");
				break;
			case 2:
				sprintf(S,"option error\n");
				break;
			case 3:
				sprintf(S,"args error\n");
				break;
		}
		printf("%s#######################################\n",S);
		s = send(client , S , sizeof(S) , 0 );
		memset(buf,0,sizeof(buf) );
		if( isexit ) break;
	}
		close(client);
		pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	int soc , bi , client;
	char *IP = argv[1] , *port = argv[2];
	int p , i = 0;
	
	sockaddr_in server , dirc;
	socklen_t len = sizeof( sockaddr_in );
	memset( &server , 0 , sizeof (server) );
	
	server.sin_family = AF_INET;
	server.sin_port = htons( atoi((const char*)port) );
	server.sin_addr.s_addr = inet_addr( IP );
	
	soc = socket( AF_INET , SOCK_STREAM , 0 );
	if( soc < 0 )
		printf("error opening socket\n");
	bi = bind(soc, (sockaddr*)&server , sizeof(server) );
	if( bi < 0 )
		printf("error on binding\n");
	listen(soc,maxcli);
	
	printf("#set up\n");
	printf("TCP Server listening on %s:%s\n\n",IP,port);
	
	pthread_t thread_id[maxcli];
	int * newsoc;
	void *ret;
	while (client = accept(soc , (sockaddr*)&dirc , &len ) ){
		puts("accept connection");
		printf("i is %d\n",i);
		newsoc =(int*)malloc(1);
		*newsoc = client;
		p = pthread_create( &thread_id[i], NULL , solve , (void*)newsoc );
		if( p < 0 )
			printf("error on pthread\n");
		i++;
	}
	
	return 0;
}

string getdata(string S, string goal){
	int pos = S.find(goal);
	pos += goal.size();cout << pos << endl;
	const char *data = S.c_str();
	string tmp = "";
	for( int i = pos ; data[i] != '\n' ; i++ ){
		tmp += data[i];
	}cout << tmp << " WW  " << endl;
	return tmp;
}

string newre(string s){
	int sz = s.size();cout << s << " in new re " << sz << endl;
	const char *S = s.c_str();
	string tmp = "";
	for( int i = sz-1 ; S[i] != ':' && i >= 0 ; --i ){
		tmp = S[i] + tmp;
	}
	tmp = "re:"+tmp;
	return tmp;
}
string newfwd(string s){
	int sz = s.size();cout << s << " in new fwd " << sz << endl; 
	const char *S = s.c_str();
	string tmp = "";
	for( int i = sz-1 ; S[i] != ':'&& i >= 0 ; --i ){
		tmp = S[i] + tmp;
	}
	tmp = "fwd:"+tmp;
	return tmp;
}

bool optest(string command , char opt){
	
	if( command == "init" ){
		if( opt == 'u' ){
			return false;
		}
	}
	else if( command == "ls" ){
		if( opt == 'u' )
			return false;
		else if( opt == 'l' )
			return false;
		else if( opt == 'a' )
			return false;
	}
	else if( command == "rm" ){
		if( opt == 'd' )
			return false;
		else if( opt == 'D' )
			return false;
	}
	else if( command == "rd" ){
		if( opt == 'r' )
			return false;
	}
	else if( command == "wt" ){
		if( opt == 'd' )
			return false;
		else if( opt == 't' )
			return false;
		else if( opt == 'c' )
			return false;
	}
	else if( command == "re" ){
		if( opt == 'c' )
			return false;
		else if( opt == 'n' )
			return false;
	}
	else if( command == "fwd" ){
		if( opt == 'd' )
			return false;
		else if( opt == 'c' )
			return false;
		else if( opt == 'n' )
			return false;
	}
	return true;
}
bool nametest(string s){
	const char *S = s.c_str();
	int sz = s.size();
	bool isgood = true;
	for( int i = 0 ; i < sz ; i++ ){
		if( !( S[i] >= 65 && S[i] <= 90 || ( S[i] == '_' || S[i] == '-' ) ) ) isgood = false;
	}
	return isgood;
}
bool rtctest(string s){
	const char *S = s.c_str();
	int sz = s.size();
	bool isgood = true;
	for( int i = 0 ; i < sz ; i++ ){
		if( !( S[i] >= 65 && S[i] <= 90 || ( S[i] == '_' || S[i] == '-' || S[i] == ':' || S[i] == '.' ||S[i] == '@' ) ) ) isgood = false;
	}
	return isgood;
}
string getacc(string s){
	const char *S = s.c_str();
	string tmp = "";
	for( int i = 0 ; S[i] != '@' ; i++ )
		tmp += S[i];
	return tmp;
}

