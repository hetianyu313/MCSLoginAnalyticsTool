```markdown
# 🎮 Minecraft 服务器日志分析系统

一个强大的 Minecraft 服务器日志分析工具，用于统计玩家登录信息、死亡记录，并生成美观的可视化 HTML 报告。

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

## ✨ 功能特性

### 📊 登录统计
- ✅ **玩家登录追踪** - 记录每个玩家的所有登录信息
- ✅ **地理位置分析** - 统计玩家从不同地点的登录次数
- ✅ **IP 地址追踪** - 记录和关联玩家使用的 IP 地址
- ✅ **坐标记录** - 保存玩家登录时的世界坐标
- ✅ **文件溯源** - 每条记录都可追溯到源日志文件和行号

### 💀 死亡统计
- ✅ **自然死亡统计** - 记录玩家自然死亡次数（`died`）
- ✅ **指令击杀统计** - 记录被指令杀死次数（`was killed`）
- ✅ **详细记录** - 包含时间、类型、文件位置等完整信息
- ✅ **排行榜** - TOP 10 死亡玩家展示

### 📈 数据可视化
- ✅ **响应式网页** - 精美的渐变配色 HTML 报告
- ✅ **多维度分析** - 6 个独立的统计视图
- ✅ **交互式表格** - 可展开详情、搜索过滤
- ✅ **数据导出** - 支持导出 JSON 格式数据

## 📋 系统要求

- **操作系统**: Windows 10/11
- **编译器**: 支持 C++17 的编译器（如 MSVC、MinGW-w64）
- **依赖**: C++ 标准库（filesystem、regex 等）

## 🚀 快速开始

### 1️⃣ 编译程序

使用 **Visual Studio**:
```bash
cl /EHsc /std:c++17 main.cpp /Fe:LogAnalyzer.exe
```

使用 **MinGW-w64**:
```bash
g++ -std=c++17 main.cpp -o LogAnalyzer.exe
```

使用 **CMake** (推荐):
```cmake
cmake_minimum_required(VERSION 3.10)
project(MinecraftLogAnalyzer)

set(CMAKE_CXX_STANDARD 17)
add_executable(LogAnalyzer main.cpp)
```

### 2️⃣ 准备日志文件

创建目录结构：
```
项目目录/
├── LogAnalyzer.exe
└── 1/                    # 日志文件夹
    ├── server-2024-01-01.log
    ├── server-2024-01-02.log
    └── ...
```

### 3️⃣ 运行程序

```bash
LogAnalyzer.exe
```

### 4️⃣ 查看结果

程序运行完成后会生成 `result.html`，用浏览器打开即可查看完整报告。

## 📖 使用说明

### 日志格式要求

程序支持解析以下格式的 Minecraft 服务器日志：

#### 登录记录
```log
  [19:51:42] [Server thread/INFO]: [PotatoIpDisplay] Player named XXXX connected from Beijing, China
[19:51:42] [Server thread/INFO]: Oliver[/192.168.1.100:12345] logged in with entity id 12345 at ([world]123.45, 64.00, -678.90)
```

#### 死亡记录
```log
[19:51:42] [Server thread/INFO]: XXX died
[17:50:54] [Server thread/INFO]: XXX was killed
```

**重要规则**:
- 玩家名必须**不包含空格**（仅支持字母、数字、下划线）
- 日志文件必须是 `.log` 扩展名
- 所有日志文件必须放在 `1/` 目录下

### 报告功能说明

生成的 HTML 报告包含 **6 个主要板块**：

#### 📊 概览统计
- 总玩家数、登录记录数、地点数、IP 数、死亡次数
- TOP 10 登录最多的玩家
- TOP 10 自然死亡最多的玩家

#### 🌍 地点统计
- 按登录次数排序的所有地点
- 可展开查看每个地点的详细登录记录
- 包含玩家名、IP、坐标、时间等信息

#### 👤 玩家统计
- 所有玩家的完整统计信息
- 可展开查看每个玩家的登录历史
- 支持搜索和筛选

#### 💀 死亡统计
- 所有玩家的死亡数据
- 区分自然死亡和指令击杀
- 可查看详细的死亡记录时间线

#### 📈 多维分析
- 按不同地点数量排序
- 展示玩家的多维度数据关联
- 最常登录地点分析

#### 📋 完整记录
- 所有登录记录的时间线
- 可搜索、过滤
- 支持导出为 JSON

## 🔧 高级功能

### 搜索和筛选

在网页顶部的搜索栏中：
- 输入玩家名、IP、地点进行实时搜索
- 使用下拉菜单按类型筛选
- 点击"重置"按钮清除筛选

### 数据导出

点击"导出数据"按钮可将所有统计数据导出为 JSON 格式，包括：
- 所有玩家的登录记录
- 所有玩家的死亡统计

导出的 JSON 结构：
```json
{
  "players": [
    {
      "name": "Oliver",
      "records": [
        {
          "time": "19:51:42",
          "location": "Beijing, China",
          "ip": "192.168.1.100",
          "world": "world"
        }
      ]
    }
  ],
  "deaths": [
    {
      "name": "Oliver",
      "naturalDeaths": 5,
      "commandKills": 2
    }
  ]
}
```

## 📊 控制台输出示例

```
开始扫描日志文件...
找到 3 个日志文件

✓ 时间: 19:51:42 | 玩家: Oliver | 位置: Beijing, China
  └─ 文件: D:\logs\1\server-2024-01-01.log:123
  └─ IP: 192.168.1.100 | 世界: world | 坐标: (123.45, 64.00, -678.90)
💀 时间: 20:15:30 | 玩家: Oliver | 类型: 自然死亡
  └─ 文件: D:\logs\1\server-2024-01-01.log:456
✓ 已处理: server-2024-01-01.log (1500 行, 45 条记录)

==================================================
处理完成: 3 个文件, 4500 行, 120 条有效记录
死亡记录: 35 条
==================================================

✓ 结果已保存到 result.html

统计摘要:
  - 总玩家数: 15
  - 总登录记录: 120
  - 不同地点数: 8
  - 不同IP数: 25
  - 总死亡次数: 35
```

## 🎨 界面预览

报告采用现代化设计：
- 🌈 **渐变配色** - 蓝紫色主题，视觉舒适
- 📱 **响应式布局** - 支持桌面和移动设备
- 🎯 **交互式组件** - 可展开详情、悬停效果
- 🏆 **排名徽章** - 前三名特殊标识

## ❓ 常见问题

### Q: 为什么某些玩家没有被统计？
**A**: 请检查：
1. 玩家名是否包含空格（不支持）
2. 日志格式是否正确
3. 日志文件是否在 `1/` 目录下

### Q: 如何处理大量日志文件？
**A**: 程序会自动处理 `1/` 目录下的所有 `.log` 文件，支持数千个文件。

### Q: 死亡统计不准确？
**A**: 确保日志中的死亡消息严格符合以下格式：
- `玩家名 died` (自然死亡)
- `玩家名 was killed` (指令击杀)
- 玩家名不能包含空格

### Q: 如何修改输出文件名？
**A**: 编辑源代码中的：
```cpp
std::ofstream html("result.html");  // 修改为你想要的文件名
```

### Q: 支持 Linux/Mac 吗？
**A**: 需要修改部分 Windows 特定代码（如 `SetConsoleOutputCP`），文件系统部分已使用跨平台的 `std::filesystem`。

## 🔍 技术细节

### 正则表达式模式

- **登录玩家名**: `Player named\s+(\w+)`
- **登录地点**: `connected from\s+(.+?)\s*$`
- **IP 地址**: `\[/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):`
- **自然死亡**: `\[Server thread/INFO\]:\s+(\S+)\s+died`
- **指令击杀**: `\[Server thread/INFO\]:\s+(\S+)\s+was killed`

### 数据结构

```cpp
struct PlayerData {
    map<string, int> locationCount;              // 地点→次数
    map<string, vector<LocationInfo>> locationDetails;  // 地点→详细记录
    vector<LoginRecord> allRecords;              // 所有登录记录
};

struct DeathData {
    int naturalDeaths;  // 自然死亡次数
    int commandKills;   // 指令击杀次数
};
```

## 📝 更新日志

### v2.0.0 (最新)
- ✨ 新增死亡统计功能
- ✨ 新增死亡详细记录板块
- ✨ 首页显示 TOP 10 死亡玩家
- 🐛 修复玩家名包含空格时的误匹配

### v1.0.0
- ✨ 基础登录统计功能
- ✨ 地点、玩家、IP 统计
- ✨ HTML 报告生成

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

AGPL3

## 💡 提示

- 定期备份日志文件
- 建议每周运行一次分析
- 大型服务器建议分批处理日志
- 可以通过修改 CSS 自定义报告样式

## 📧 联系方式

如有问题或建议，请通过 Issue 反馈。
`htyjx@hotmail.com`

---

**⭐ 如果这个项目对你有帮助，请给个 Star！**
