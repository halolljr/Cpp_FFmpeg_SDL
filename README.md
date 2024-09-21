note:不提供原视频，请按程序内容来设置视频输入

另外：注意配置好FFmpeg

还在学习（因此搁置了流媒体服务器plus版的学习与更新）

**2024.9.18**

​         更新了yuv播放器(注意yuv的宽高，播放失败很可能就是你的yuv宽高没在程序中对应好)

**ffmpeg -i 你的源文件 -an -c:v rawvideo -pix_fmt yuv420p 输出文件.yuv**

2024 9. 21

​         更新了pcm播放器（注意pcm的原始参数）

ffmpeg -i 你的源文件 -vn -ar 44100 -ac 2 -f s16le 输出文件.pcm