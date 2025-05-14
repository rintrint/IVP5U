# IVP5U (New IM4U)  
PMX and VMD loader for UnrealEngine5  
Support UE5.5  

## UE5 MMD教程  
UE5.5 MMD教程 替代Blender渲染 VRM4U IVP5U NexGiMa FFmpeg  
https://www.bilibili.com/video/BV1EbEzzrED9/  
UE5.2 动画製作: VRM4U导入PMX模型 + IVP5U导入VMD动作  
https://www.bilibili.com/video/BV17p4y1K7MM/  
UE5.2 动画製作: 使用 IVP5U 导入MMD模型、动作和镜头  
https://www.bilibili.com/video/BV1Ju4y197Pz/  

##  NexGiMa烘焙物理  
NexGiMa下载  
https://sites.google.com/view/nexgima/Home#h.p_zUEhIi0xDZD8  
NexGiMa教程  
https://www.bilibili.com/video/BV1Hz42167oE/  

## 注意事项  
Project Settings开启Support 16-bit Bone Index  
未开启会导致模型直接不显示，变成空气  
默认的8-bit Bone Index仅支持256根骨骼，MMD模型容易超过  

使用VRM4U导入模型后，需要重启UE5  
未重启就导入VMD会导致动画鬼畜，原因不明，不重启有时还会导致UE5卡死  

设置Movie Pipeline CLI Encoder  
```
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
https://github.com/BtbN/FFmpeg-Builds/releases 选择ffmpeg-master-latest-win64-gpl-shared.zip  
https://www.gyan.dev/ffmpeg/builds 选择ffmpeg-git-full.7z  

## 插件的行为  
骨骼关键帧全部导入  
表情关键帧如果只有一个且值为0，则跳过，其馀全部导入  

## Reference  
- https://github.com/bm9/IM4U  
- https://github.com/axilesoft/IM-for-UE5  
- https://github.com/NaN-Name-bilbil/IVP5U  

## 插件开发  
### TODO  
更新过时的API  
调查Sequencer关键帧形状和颜色没有正确显示的问题  

### 已完成  
优化两大瓶颈  
优化VMDLoaderBinary，从50秒变为1秒  
优化ImportAnimations，从50秒变为4秒  

不负责任的优化顺序树  
```
FactoryCreateBinary 100秒变为5秒
    |
    ├── VMDLoaderBinary 50秒变为1秒
    |
    └── ImportAnimations 50秒变为4秒
        |
        ├── AnimDataController操作 48.5秒变为3.5秒
        |
        └── interpolateBezier 1.5秒变为0.5秒
```

### 代码风格  
使用.clang-format文件  
文件是从 https://github.com/TensorWorks/UE-Clang-Format 下载的  
并做出以下改动  
```
AlignConsecutiveDeclarations: false
```

### 导入VMD过程流程图  
IVP5U文件夹  
```
FactoryCreateBinary
    |
    ├── VMDLoaderBinary (耗时操作！完整解析VMD文件)
    |
    ├── 检查是否为相机动画
    |   |
    |   └── (如果是) 显示警告讯息 (未实现，在插件的另一个功能里：MMDCameraImporter文件夹)
    |
    └── (如果不是) 导入骨骼和变形动画
        |
        ├── GetVMDImportOptions (显示导入选项对话框，等待用户操作)
        |
        ├── 检查导入选项
        |
        ├── 如果是新建AnimSequence
        |   |
        |   └── ImportAnimations
        |       |
        |       ├── 创建新的AnimSequence
        |
        └── 如果是向现有AnimSequence添加变形数据
            |
            └── AddtionalMorphCurveImportToAnimations
                |
                └── ImportMorphCurveToAnimSequence
```
MMDCameraImporter文件夹  
```
ImportVmdCamera
    |
    ├── 检查是否为相机动画文件
    |
    ├── 创建场景相机和相机中心
    |   |
    |   └── 设置初始相机参数和附加关系
    |
    └── ImportVmdCameraToExisting
```

### 导入参数说明  

在插件早期版本中包含的重要信息，保留在此作为参考：  

#### 导入参数说明  
当前有效的参数包括：  
- Skeleton Asset（必需：与动画相关联）  
- SkeletalMesh Asset（可选：Animation关联到MorphTarget。NULL时，MorphTargetSkip）  
- 动画资源（执行仅将Morph添加到现有资源（非空值）的过程。在空值下创建包含Bone和Morph的新资源）  
- DataTable（MMD2UE5Name）Asset（任意：在NULL以外读取时，用MMD = UE5替换Bone和MorphName，执行导入。需要事先以CSV形式导入或新建）  
- MmdExtendAsset（可选：在NULL以外从VMD生成AnimSeq资产时，从Extend参照IK信息进行计算时使用。必须事先导入模型或手动生成资产）  

注意：新Asset生成因IK等未对应而不推荐。仅支持追加Morph。  

#### VMD目标模型信息警告  
注意：运动数据取入信息：  
此VMD是为特定模型创建的文件。  

对于模型运动，仅捕获具有相同骨骼名称的数据。  
如果包含名称与模型侧骨骼名称不同的相同骨骼，则  
预先创建转换表（MMD2UE5NameTableRow），  
可通过在InportOption画面中指定进行导入。  

#### 导入选项参数检查警告  
注意："导入"选项的参数检查：  

强制要求(必须)  
- 骨架资源：选择目标骨架。  
- 如果为NULL，则表示导入错误。  

可选(任意)  
- 骨骼网格资源：选择目标骨骼网格。  
- 但是，SkellMesh包括骨架。(但是，网格必须选择相同的骨架)  
- 如果为NULL，则跳过导入变形曲线。(未捕获变形)  

如果参数检查不通过，则需要重试导入选项。  

### VMD文件结构说明  

1. **VMD文件结构说明**：  
   ```
   // VMD文件格式说明：
   // 1. 使用"Vocaloid Motion Data 0002"作为文件魔数标识
   // 2. 包含骨骼、变形器、相机、灯光、阴影和属性等多种关键帧数据
   // 3. 所有数据都使用小端序存储
   ```

2. **插值数据解释**：  
   ```
   // VMD插值数据格式说明：
   // 相机的插值数据(Interpolation)是一个24字节数组，用于贝塞尔曲线控制
   // 索引0-3：X位置的插值控制点
   // 索引4-7：Y位置的插值控制点
   // 索引8-11：Z位置的插值控制点
   // 索引12-15：旋转的插值控制点(所有旋转轴共用)
   // 索引16-19：距离的插值控制点
   // 索引20-23：视角(FOV)的插值控制点
   ```

3. **切线处理逻辑**：  
   ```
   // 切线计算逻辑说明：
   // 1. 原始VMD使用贝塞尔曲线控制点，值范围是0-127
   // 2. 需要除以127转换为0-1范围的值
   // 3. 对于到达切线，需要使用(1-值)作为最终数值
   // 4. UE使用的是切线方向和权重，需要转换为合适的表示
   // 5. 切线方向 = 切线Y值 / (切线X值 * 帧率)
   // 6. 切线权重 = 切线向量的长度
   ```

4. **相机变换说明**：  
   ```
   // 相机使用两个Actor协同工作：
   // 1. CameraCenter: 负责位置和旋转，相当于相机的锚点或目标点
   // 2. Camera: 附加到CameraCenter，相对位置由Distance决定
   // 这种结构允许相机围绕一个点旋转，模拟MMD中相机看向目标点的行为
   ```

5. **轴映射解释**：  
   ```
   // 轴映射说明：
   // MMD和UE使用不同的坐标系，需要映射:
   // MMD: Y轴向上，Z轴向前，X轴向右
   // UE:  Z轴向上，X轴向前，Y轴向右
   // 默认映射: 
   // - MMD的Z映射到UE的X (前方)
   // - MMD的X映射到UE的Y (右方)
   // - MMD的Y映射到UE的Z (上方)
   // 可通过ImportVmdSettings中的AxisMapping*参数自定义
   ```

6. **关于相机切割的说明**：  
   ```
   // 相机切割(CameraCut)说明：
   // 1. 用于检测相机的突变，例如镜头切换
   // 2. 检测两个相邻帧之间的大变化：距离、位置、旋转、视角等
   // 3. 根据相邻帧间隔和数值变化来决定是否为切割点
   // 4. CameraCut决定了多个相机Actor之间的转换时机
   ```

7. **焦距计算公式**：  
   ```
   // 焦距计算公式：
   // 焦距 = (感应器宽度 / 2) / tan(视角 / 2)
   // 其中视角需要先转换为弧度
   // 最终焦距还会乘以用户指定的FocalLengthScale因子
   ```
