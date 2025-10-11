# IVP5U (New IM4U)

PMX and VMD loader for UnrealEngine5  
Support UE5.5, 5.6, 5.7

| UE5版本   | 插件版本     |
|-----------|-------------|
| UE5.5     | IVP5U_UE5.5 |
| UE5.6     | IVP5U_UE5.6 |
| UE5.7     | main分支    |

建议搭配VRM4U使用(最佳组合):

- **VRM4U**:导入PMX
- **IVP5U**:导入VMD

## UE5 MMD教程

UE5.5 MMD教程 替代Blender渲染 VRM4U IVP5U NexGiMa FFmpeg  
<https://www.bilibili.com/video/BV1EbEzzrED9/>  
UE5.2 动画製作: VRM4U导入PMX模型 + IVP5U导入VMD动作  
<https://www.bilibili.com/video/BV17p4y1K7MM/>  
UE5.2 动画製作: 使用 IVP5U 导入MMD模型、动作和镜头  
<https://www.bilibili.com/video/BV1Ju4y197Pz/>  

## MMDBridge烘焙物理

MMDBridge下载  
<https://github.com/rintrint/mmdbridge>  
MMDBridge教程  
<https://www.bilibili.com/opus/1102730546871533640>  
<https://github.com/rintrint/mmdbridge/blob/master/docs/how_to_use.md>  

## 注意事项

Project Settings开启Support 16-bit Bone Index  
未开启会导致模型直接不显示，变成空气  
默认的8-bit Bone Index仅支持256根骨骼，MMD模型容易超过  

使用VRM4U导入模型后，需要重启UE5  
未重启就导入VMD会导致动画鬼畜，原因不明，不重启有时还会导致UE5卡死  

设置Movie Pipeline CLI Encoder  

```text
ffmpeg.exe
Run 'MovieRenderPipeline.DumpCLIEncoderCodecs' in Console to see available codecs.
av1_nvenc
libopus
mp4
-hide_banner -y -loglevel error -init_hw_device vulkan -thread_queue_size 32768 {VideoInputs} {AudioInputs} -acodec {AudioCodec} -vcodec {VideoCodec} {Quality} -vf "libplacebo=colorspace=bt709:color_primaries=bt709:color_trc=iec61966-2-1:range=tv:format=yuv444p16le" -pix_fmt yuv420p -g 60 -c:a libopus -b:a 512k -ar 48000 -movflags +faststart -flags +cgop -coder cabac {AdditionalLocalArgs} "{OutputPath}"
-r {FrameRate} -f concat -safe 0 -i "{InputFile}"
-f concat -safe 0 -i "{InputFile}"
-qp 60
-qp 50
-qp 40
-qp 30
```

ffmpeg下载(二选一)，并设置环境变数  
<https://github.com/BtbN/FFmpeg-Builds/releases> 选择ffmpeg-master-latest-win64-gpl-shared.zip  
<https://www.gyan.dev/ffmpeg/builds> 选择ffmpeg-git-full.7z  

## 插件的行为

骨骼关键帧全部导入  
表情关键帧如果只有一个且值为0，则跳过，其馀全部导入  

## Reference

- <https://github.com/bm9/IM4U>
- <https://github.com/axilesoft/IM-for-UE5>
- <https://github.com/NaN-Name-bilbil/IVP5U>
