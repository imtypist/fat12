# fat12

### 文件说明

`image/` : fat12格式测试文件

`DiskLib.[h|lib|dll]` : 动态链接库文件

`fat12/` : FAT12动态链接库目录

`fat12_test/` : 测试程序目录

### fat12引导扇区字段说明

| 名称             | 偏移(BYTE) | 长度(BYTE) | 内容                         | 软盘参考值                  |
| -------------- | -------- | -------- | -------------------------- | ---------------------- |
| BS_jmpBoot     | 0        | 3        | -                          | jmp LABEL_START && nop |
| BS_OEMName     | 3        | 8        | 厂商名                        | 'Forrest Y'            |
| BPB_BytsPerSec | 11       | 2        | 每扇区字节数                     | 0x200(512字节)           |
| BPB_SecPerClus | 13       | 1        | 每簇扇区数                      | 0x01                   |
| BPB_RsvdSecCnt | 14       | 2        | Boot记录占用多少                 | 0x01                   |
| BPB_NumFATs    | 16       | 1        | 共有多少FAT表                   | 0x02                   |
| BPB_RootEntCnt | 17       | 2        | 根目录文件数最大值                  | 0xE0(224)              |
| BPB_TotSec16   | 19       | 2        | 扇区总数                       | 0xB40(2280)            |
| BPB_Media      | 21       | 1        | 介质描述符                      | 0xF0                   |
| BPB_FATSz16    | 22       | 2        | 每FAT扇区数                    | 0x09                   |
| BPB_SecPerTrk  | 24       | 2        | 每磁道扇区数                     | 0x12                   |
| BPB_NumHeads   | 26       | 2        | 磁头数                        | 0x02                   |
| BPB_HiddSec    | 28       | 4        | 隐藏扇区数                      | 0                      |
| BPB_TotSec32   | 32       | 4        | 如果BPB_TotSec16是0，由这个值记录扇区数 | 0xB40(2280)            |
| BS_DrvNum      | 36       | 1        | 中断13的驱动器号                  | 0                      |
| BS_Reserved1   | 37       | 1        | 未使用                        | 0                      |
| BS_BootSig     | 38       | 1        | 扩展引导标记                     | 0x29                   |
| BS_VolD        | 39       | 4        | 卷序列号                       | 0                      |
| BS_VolLab      | 43       | 11       | 卷标                         | 'OrangeS0.02'          |
| BS_FileSysType | 54       | 8        | 文件系统类型                     | 'FAT12'                |
| 引导代码           | 62       | 448      | 引导代码、数据及其他填充字符等            |                        |
| 结束标志           | 510      | 2        |                            | 0xAA55                 |

