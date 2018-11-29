#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <time.h>

void addSlash(char *path)
{
	if(sizeof(path) - strlen(path) < 2)
		return;
	int i = strlen(path);
	if(path[i-1] == '/')
		return;
	path[i] = '/';
	path[i+1] = '\0';
}

int isDir(char *path)
{
	struct stat st;
	int ret = stat(path,&st);
	if(ret == -1) return 0;
	if(S_ISDIR(st.st_mode))
		return 1;
	return 0;
}
/*  根据起始路径和当前路径，生成相对路径
 *  @param startPath 起始路径，绝对路径
 *  @param currentPath 当前路径，绝对路径
 *  @param relativePath 相对路径，该字符开辟空间应大于1024，同时应该以 / 为开始字符
 */
void getUrlPath(char *startPath,char *currentPath,char *relativePath)
{
	int i = 0;
	while(startPath[i] != '\0')
	{
		if(startPath[i] != currentPath[i])
			break;
		i++;
	}
	int j = 1;
	while(currentPath[i] != '\0' && j < 1024)
	{
		relativePath[j++] = currentPath[i++];
	}
	relativePath[j] = '\0';
}

void copy(int fd1,int fd2)
{
	char buffer[4096];
	while(1)
	{
		int len = read(fd2,buffer,4096);
		if(len <= 0)
			return;
		write(fd1,buffer,len);
		if(len != 4096)
			break;
	}
	
}
//添加上级目录链接到html文件
void addParentInfo(int fd, char *nowPath)
{
	int l = strlen(nowPath)-2;
	if(l <= 0)
		return;
	while(l > 0)
	{
		if(nowPath[l] == '/')
		{
			nowPath[l+1] = '\0';
			break;
		}
		l--;
	}
	char allData[2048] = {0};
	sprintf(allData,
			"<tr>\n\t<td>\n\t\t<a href=\"%s\">\n\t\t\t..\n\t\t</a>\n\t</td>\n\t<td align=\"right\">\n\t\t&nbsp;\n\t</td>\n\t<td align=\"right\">\n\t\t-\n\t</td>\n</tr>\n"
			,nowPath);
	write(fd,allData,strlen(allData));
}

//添加文件表项到html
//@param fd html文件 文件描述符
//@param type 是否为文件，1为文件，否则为目录
//@param path 文件路径
//@param url 文件https url
//@param name 文件名
void addFileInfo(int fd,int type,char * path,char * url,char *name)
{
	struct stat st;
	int ret = stat(path,&st);
	if(ret == -1) return;
	//获取修改时间
	time_t t = st.st_mtime;
	struct tm  sttm;
	char szBuf[64] = {0};
	sttm = *localtime(&t);
	strftime(szBuf, 64, "%Y-%m-%d %H:%M", &sttm);
	//处理文件大小和文件名
	char len[10] = "-";	//文件大小
	char newName[1024];	//文件名
	strcpy(newName,name);
	if(type == 1)
	{
		//文件
		if(st.st_size > 1024 * 1024 * 1024)
			sprintf(len,"%.2fGB",(double)st.st_size/1024.0/1024.0/1024.0);
		else if(st.st_size > 1024 * 1024)
			sprintf(len,"%.2fMB",(double)st.st_size/1024.0/1024.0);
		else if(st.st_size > 1024)
			sprintf(len,"%.2fKB",(double)st.st_size/1024.0);
		else
			sprintf(len,"%ldB",st.st_size);
	}
	else
	{
		//目录，文件名后边加'/'
		addSlash(newName);
	}
	char allData[2048] = {0};
	sprintf(allData,
			"<tr>\n\t<td>\n\t\t<a href=\"%s\">\n\t\t\t%s\n\t\t</a>\n\t</td>\n\t<td align=\"right\">\n\t\t%s\n\t</td>\n\t<td align=\"right\">\n\t\t%s\n\t</td>\n</tr>\n"
			,url,newName,szBuf,len);
	write(fd,allData,strlen(allData));
}
//生成html文件
//@param startPath 选定的根目录，应当以'/'结尾
//@param currentPath 当前操作的目录，应当以'/'结尾
void createHtml(char *startPath,char *currentPath)
{
	DIR *dp = opendir(currentPath);
	if(dp==NULL)
		return;
	//要生成的html文件路径
	char htmlFile[1024];
	strcpy(htmlFile,currentPath);
	strcat(htmlFile,"index.html");
	//文件存在则截断为0，不存在则创建文件
	int fd = open(htmlFile, O_WRONLY | O_CREAT | O_TRUNC);
	if(fd == -1)
		return;
	//复制第一段html内容
	int fd2 = open("./html1", O_RDONLY);
	if(fd2 == -1)
	{
		close(fd);
		return;
	}
	copy(fd,fd2);
	close(fd2);
	//生成标题
	char title[1024];
	char relativePath[1024];
	relativePath[0] = '/';
	relativePath[1] = '\0';
	getUrlPath(startPath,currentPath,relativePath);
	sprintf(title,"\n<h2>Index of %s </h2>\n",relativePath);
	write(fd,title,strlen(title));
	//复制第二段html内容
	fd2 = open("./html2", O_RDONLY);
	if(fd2 == -1)
	{
		close(fd);
		return;
	}
	copy(fd,fd2);
	close(fd2);
	//如果不是根目录，则生成上级目录链接
	if(strcmp(startPath,currentPath)!=0){
		char url[2048] = {0};
		sprintf(url,"http://download.zuozl.com%s",relativePath);
		addParentInfo(fd,url);
	}
	//遍历目录
	struct dirent *dir;
	while((dir=readdir(dp))!=NULL)
	{
		//隐藏文件跳过
		if(dir->d_name[0] == '.') continue;
		//index.html文件跳过
		if(strcmp(dir->d_name,"index.html")==0) continue;
		char url[2048] = {0};
		sprintf(url,"http://download.zuozl.com%s%s",relativePath,dir->d_name);
		char filePath[2048];
		strcpy(filePath,currentPath);
		strcat(filePath,dir->d_name);
		if(isDir(filePath))
		{
			addSlash(filePath);
			addSlash(url);
			addFileInfo(fd,0,filePath,url,dir->d_name);
			createHtml(startPath,filePath);
		}
		else
		{
			addFileInfo(fd,1,filePath,url,dir->d_name);
		}
	}
	//复制第三份html内容到文件中
	fd2 = open("./html3", O_RDONLY);
	if(fd2 == -1)
	{
		close(fd);
		return;
	}
	copy(fd,fd2);
	close(fd2);
	//修改文件权限，其他人可读 664
	mode_t mod;
	mod =  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	fchmod(fd,mod);
	close(fd);
	closedir(dp);
}

//生成md5文件
//@param path 选定的根目录，应当以'/'结尾
void createMd5sumFile(char *path)
{
	DIR *dp = opendir(path);
	if(dp==NULL)
		return;

	int re = chdir(path);
	if(re == -1) return;

	int p = fork();
	if(p == 0)
	{
		if(execlp("rm","rm","-rf","md5sum.txt",NULL) < 0)
			exit(0);
	}
	else
	{
		wait(&p);
	}
	//遍历目录
	
	struct dirent *dir;
	while((dir=readdir(dp))!=NULL)
	{
		//隐藏文件跳过
		if(dir->d_name[0] == '.') continue;
		//index.html文件跳过
		if(strcmp(dir->d_name,"index.html")==0) continue;
		if(strcmp(dir->d_name,"md5sum.txt")==0) continue;
		int re = chdir(path);
		if(re == -1) continue;
		char filePath[2048] = {0};
		strcpy(filePath,path);
		strcat(filePath,dir->d_name);
		
		if(isDir(filePath))
		{
			addSlash(filePath);
			createMd5sumFile(filePath);
		}
		else
		{
			int childpid = fork();
			if(childpid == 0)
			{
				int fd = open("md5sum.txt",O_WRONLY | O_CREAT | O_APPEND);
				mode_t mod;
				mod =  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
				fchmod(fd,mod);
				dup2(fd,STDOUT_FILENO);
				if(execlp("md5sum","md5sum",dir->d_name,NULL) < 0)
				{
					close(fd);
					exit(0);
				}
				else
					close(fd);
			}
			else
			{
				wait(&childpid);
			}
		}

	}
	closedir(dp);
}

int main(int argc,char **argv)
{
	if(argc < 2)
	{
		printf("Please input create path.");
		return 0;
	}
	char path[1024];
	strcpy(path,argv[1]);
	addSlash(path);
	//生成MD校验和文件
	createMd5sumFile(path);
	//生成html文件
	createHtml(path,path);
	return 0;
}