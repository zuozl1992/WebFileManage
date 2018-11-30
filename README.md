# WebFileManage
## 网页文件管理
* 可以生成一个web的文件管理页面
* 使用$gcc updateWebFileManage.c -o updateWebFileManage编译
* 使用root用户运行
*  $ ./updateWebFileManage -all path
* 可在path目录下生成一套index.html及md5sum.txt文件
* * -md5 只生成md5文件
* * -html 只生成html文件
* * -all 生成所有
* md5生成需要时间，可以单独将产生变化的文件夹更新md5文件，然后将所有的文件夹更新html文件：
* * $ ./updateWebFileManage -md5 path/changedPath
* * $ ./updateWebFileManage -html path
