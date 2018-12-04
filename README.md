# WebFileManage
## 网页文件管理
* 可以生成一个web的文件管理页面
* 使用$gcc updateWebFileManage.c -o updateWebFileManage编译
* 使用root用户运行
*  $ ./updateWebFileManage -d <path>
* 可在path目录下生成一套index.html及md5sum.txt文件
	* -d --dir <path>		操作目录。
	* -w --web			只生成html文件。
	* -m --md5			只生成md5文件。
	*    --no_web_rec <startPath>	只在顶层目录生成web文件，不遍历子目录。
	*    --no_md5_rec		只在顶层目录生成md5文件，不遍历子目录。
	* -h --help			帮助。
