# WebFileManage
## 网页文件管理
* 可以生成一个web的文件管理页面
* 使用$gcc updateWebFileManage.c -o updateWebFileManage编译
* 使用root用户运行
*  $ ./updateWebFileManage -d path
* 可在path目录下生成一套index.html及md5sum.txt文件
* * -d --dir 操作目录
* * -m --md5 只生成md5文件
* * -w --web 只生成html文件
* * -n --no_rec 只操作当前目录，不遍历子目录
