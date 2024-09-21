写在前面：FFmpeg_raw文件夹中是FFmpeg源代码，未经过编译的(如何编译详看：[【秒懂音视频开发】14_编译FFmpeg - M了个J - 博客园 (cnblogs.com)](https://www.cnblogs.com/mjios/p/14633516.html))

​					FFmpeg_shared_build文件夹中是已经经过编译的了，会缺少一些功能

![屏幕截图 2024-09-18 160313](images\一\屏幕截图 2024-09-18 160313.png)

![屏幕截图 2024-09-18 160421](images\一\屏幕截图 2024-09-18 160421.png)

# FFmpeg

## 一、FFmpeg处理音视频流程

### ![099734a9f5d063d23d04249cbc50c13](images\一\099734a9f5d063d23d04249cbc50c13.jpg)

## 二、FFmpeg基本信息查询命令

## ![caebc170ce8cf128f7cb05cd0cb8fe1](D:\Alearn\ACPP\store\YSP\FFmpeg-Learn\images\二\caebc170ce8cf128f7cb05cd0cb8fe1.jpg)

### 2.1 基本信息查询命令

![056d9afa2eb17f61e446af937aa4535](images\二\056d9afa2eb17f61e446af937aa4535.jpg)

### 2.2 录制命令

#### 2.1.1 录制视频

![a4a8a26dcedbe554eae7018bb1671f9](images\二\a4a8a26dcedbe554eae7018bb1671f9.jpg)

-f：指定XX库来录制

-i：{0：摄像头；1：屏幕；etc}

-r：帧率

#### 2.1.2 录制音频

![8a2c3e27a564fd89daf544013219e94](images\二\8a2c3e27a564fd89daf544013219e94.jpg)

-i :0 {0：音频设备1；etc}

#### 2.1.3 如何同时录制音频和视频呢？

### 2.3 分解/复用命令（多媒体格式之间转化）

![9b4a92e801c9574e0bfa54c178ca0d3](images\二\9b4a92e801c9574e0bfa54c178ca0d3.jpg)

#### 2.3.1 MP4（XX）转flv（XX）

#### ![1e78e5141264f6154e263a3d78c1aa0](images\二\1e78e5141264f6154e263a3d78c1aa0.jpg)

#### 2.3.2 单独将视频流转化

![36c0d2150b77ceeecd4433342606719](images\二\36c0d2150b77ceeecd4433342606719.jpg)

#### 2.3.3 单独提取音频

![af12d10c6408876379b8fa23bc242f0](images\二\af12d10c6408876379b8fa23bc242f0.jpg)

### 2.4 处理原始数据命令（视频原始数据：YUV；音频原始数据：PCM)

#### 2.4.1 FFmpeg提取YUV数据

![f865cc930bb7ffb98235c99ef653d11](images\二\f865cc930bb7ffb98235c99ef653d11.jpg)

-i：输入的数据

-an：{a:audio，音频；n:no}

-c:v：编码方式

-pix_fmt：像素格式{yuv：4:4:4；yuv：4:2:2；yuv：4:2:0}；最常用的4:2:0

#### 2.4.2 FFmpeg提取PCM数据

![5d003954343e1730ab257395a2d3705](images\二\5d003954343e1730ab257395a2d3705.jpg)

-i：

-vn：不包含视频流

-ar：{a:audio，音频；r：rate，采样率（44.1k；48k；32k；16k）}

-ac  2：{a：audio；c：channel，单声道，双声道（2），立体声，环绕立体声etc}（**有空格**）

-f：数据存储格式{s16le：{s：有符号；16：每一位数值用16位表示；l：小头；e：}}

### 2.5 裁剪与合并命令

#### 2.5.1 裁剪命令

![f14acf6fa7a3aeae47b7075ffa5fcb0](images\二\f14acf6fa7a3aeae47b7075ffa5fcb0.jpg)

-ss:从哪里开始裁剪
-t：裁剪多少秒

#### 2.5.2 合并命令

![f34e713b3b1b795fa41eb08765254e6](images\二\f34e713b3b1b795fa41eb08765254e6.jpg)

-f concat：拼接
-i 文件列表.txt :要拼接的文件（file＋视频切片文件.ts）

**如何将多个非ts切片视频（如mp4）合并成一个视频？**

### 2.6 图片/视频互转命令

#### 2.6.1 视频转化为图片

![76fbb91e07efb994f7cd0199dea7605](images\二\76fbb91e07efb994f7cd0199dea7605.jpg)

-i：

-r：帧率，这里是每秒钟转为一张图片

-f：转化的图片格式

image-%3d.jpeg ：动态图片名称{0~n}

#### 2.6.2 图片转化为视频

![c14f3000d30bc3be4e4e4d3c3214f1e](images\二\c14f3000d30bc3be4e4e4d3c3214f1e.jpg)

### 2.7 直播相关命令（推/拉流）

#### 2.7.1 直播推流

![814326ddaa154b320560f1ff0e8fa19](images\二\814326ddaa154b320560f1ff0e8fa19.jpg)

-re：减慢帧率速度

-i：推出去的视频流

-c：音视频编解码，这里是copy（-c:a,-c:v）

-f：推出去的文件格式

rtmp://server/live/streamName ：rtmp服务器地址

#### 2.7.2 直播拉流

#### ![814326ddaa154b320560f1ff0e8fa19](images\二\814326ddaa154b320560f1ff0e8fa19.jpg)

-i：拉流的地址

-c：音视频是否重新编码，copy不需要

dump.flv（**拉流的视频格式要与保存的一样的**）

### 2.8 滤镜命令

![099734a9f5d063d23d04249cbc50c13](images\一\099734a9f5d063d23d04249cbc50c13.jpg)

![b11900b5df18d85fe66e03799521b6b](images\二\b11900b5df18d85fe66e03799521b6b.jpg)

-vf：视频滤镜名字

crop：crop滤镜所需的参数

-c:v：视频编码器

-c:a：音频编码器（这里不对音频做处理）

**如何为视频添加水印呢？**

## 三、FFmpeg初级开发

![6f000c105f320be4260e9e2950dbd48](images\三\6f000c105f320be4260e9e2950dbd48.jpg)

![8d2473f1de1a83894c6aa365e15f7e2](images\三\8d2473f1de1a83894c6aa365e15f7e2.jpg)

### **Linux系统下编译的时候，对于外来库，可以：**

![b2f99ec3aa7408d98c4101b7fa647f0](images\三\b2f99ec3aa7408d98c4101b7fa647f0.jpg)

### 3.1 FFmpeg日志系统

#### 3.1.1 如何打印日志

![23dc23761d3581d68b7b7da59970943](images\三\23dc23761d3581d68b7b7da59970943.jpg)

av_log_set_level()：设置日志的等级，譬如这里在DEBUG日志以上的日志信息都可以被打印

av_log()：{1：；2：打印的日志信息级别；3：；4：}

#### 3.1.2 常用的日志级别

![b82baa33f11f09903a77451f802258c](images\三\b82baa33f11f09903a77451f802258c.jpg)

**AV_LOG_DEBUG最低级别**

### 3.2 avpriv_io_delete && avpriv_io_move（删除文件和重命名文件，非FFmpeg公开API）

![b84b6bba7782288908eb84b0f629aab](images\三\b84b6bba7782288908eb84b0f629aab.jpg)

![05324a7ec2336b7dc6eb70f4bd37052](images\三\05324a7ec2336b7dc6eb70f4bd37052.jpg)

### 3.3 操作目录函数

![cb3dfc7378c9a248233fb0dee956de6](images\三\cb3dfc7378c9a248233fb0dee956de6.jpg)

![5dfb0c611eec69e26c30b1428ae73b2](images\三\5dfb0c611eec69e26c30b1428ae73b2.jpg)

![06b54a29026c7096ccc1f14be50d4b0](images\三\06b54a29026c7096ccc1f14be50d4b0.jpg)

![8dba0724e2bfd5447eeae0455057ab3](images\三\8dba0724e2bfd5447eeae0455057ab3.jpg)

### 3.4 处理多媒体文件

#### 3.4.1 多媒体文件的基本概念

![f701849d81a21073505a7ba99cd45c9](images\三\f701849d81a21073505a7ba99cd45c9.jpg)

![c153fac118472e696a6857d4c2e335c](images\三\c153fac118472e696a6857d4c2e335c.jpg)

3.4.2 几个重要的文件结构体

![58cd3a3c76eeb9ec401a19b72251605](images\三\58cd3a3c76eeb9ec401a19b72251605.jpg)

#### 3.4.3 FFmpeg操作流数据的基本流程

![a5c6fd5b60b03e13d41b6bcc77fa6d3](images\三\a5c6fd5b60b03e13d41b6bcc77fa6d3.jpg)

### 3.5 FFmpeg初级开发Demo（av_primary.sln）

#### 3.5.1 从视频流抽取音频

#### 3.5.2 从视频流抽取视频

#### 3.5.3 视频格式转封装

#### 3.5.4 将一个视频的音频和另一个视频的视频合为一个文件

## 四、FFmpeg中级开发（FFmpeg编解码，av_primary.sln）

![屏幕截图 2024-09-07 101909](images\四\屏幕截图 2024-09-07 101909.png)

![屏幕截图 2024-09-07 102008](images\四\屏幕截图 2024-09-07 102008.png)

![屏幕截图 2024-09-07 102342](images\四\屏幕截图 2024-09-07 102342.png)

![屏幕截图 2024-09-07 102448](images\四\屏幕截图 2024-09-07 102448.png)

![屏幕截图 2024-09-07 102604](images\四\屏幕截图 2024-09-07 102604.png)

### 使用FFmpeg进行初级开发

## 五、SDL

![屏幕截图 2024-09-12 224032](images\五\屏幕截图 2024-09-12 224032.png)

#### 5.1 渲染SDL窗口

**SDL_Init()返回值为0说明成功**

![屏幕截图 2024-09-12 234237](images\五\屏幕截图 2024-09-12 234237.png)

![7e3fb19f50907b94df8c374e5693b3d](images\五\7e3fb19f50907b94df8c374e5693b3d.jpg)

#### 5.2 SDL事件

##### 5.2.1 SDL事件基本原理

![d7062d3e9cf651d6b6d9b9d6c2e9026](images\五\d7062d3e9cf651d6b6d9b9d6c2e9026.jpg)

##### 5.2.2 SDL事件种类

![5b9347f0ec7838365c690b04d4d1a5b](images\五\5b9347f0ec7838365c690b04d4d1a5b.jpg)

##### 5.2.3 SDL处理事件的方式

![bb9b407058bd032391d2a536c915ca2](images\五\bb9b407058bd032391d2a536c915ca2.jpg)

其中SDL_WaitEvent还可以设置等待时间

#### 5.3 SDL先渲染纹理再绘制到窗口

![d944072cfb3033fcbb2a3c6bde05286](images\五\d944072cfb3033fcbb2a3c6bde05286.jpg)

![4ca404023ed56ae07b6c82fefe91215](images\五\4ca404023ed56ae07b6c82fefe91215.jpg)

![25c963890f63a273eea04886a8997b5](images\五\25c963890f63a273eea04886a8997b5.jpg)

![c5c79347a3c105296bad1a1792977ab](images\五\c5c79347a3c105296bad1a1792977ab.jpg)

**为什么不直接绘制在窗口而是要绘制在纹理再绘制在窗口**

![微信截图_20240913203702](images\五\微信截图_20240913203702.png)

![微信截图_20240913203730](images\五\微信截图_20240913203730.png)

#### 5.4 SDL线程

![d17c9c5d6df0931e8608decaa074011](images\五\d17c9c5d6df0931e8608decaa074011.jpg)

#### 5.5 实现YUV播放器(av_middle_sdl.sln)

**注意大坑：**

![屏幕截图 2024-09-18 205244](images\五\屏幕截图 2024-09-18 205244.png)

#### 5.6 实现pcm播放器

![d2cfedeaa6a57ebab404dd47ab5a47f](images\五\d2cfedeaa6a57ebab404dd47ab5a47f.jpg)

![dd62ec2a88dc89c8cb545ef55927c73](images\五\dd62ec2a88dc89c8cb545ef55927c73.jpg)

![98704ee3afbfd0f217d40cffd54a3b6](images\五\98704ee3afbfd0f217d40cffd54a3b6.jpg)

**SDL_OpenAudio()返回0代表打开成功**
