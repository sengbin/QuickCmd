# QuickCmd
控制台快捷命令工具

QuickCmd 允许你在可执行程序所在目录定义一组“命令映射”（name -> shell command），然后通过 `qcmd <name>` 直接在控制台执行对应的命令。工具同时提供添加、删除、列出和帮助等常用操作，适合将复杂或常用的构建/发布命令封装为快捷命令。

## 主要功能

- 通过 `qcmd <name>` 执行在配置文件中定义的命令映射
- `qcmd list` 列出所有映射
- `qcmd add name=command` 添加或更新映射（命令可包含空格）
- `qcmd remove name` 删除映射（别名：`rm`/`del`）
- 若配置文件不存在，会在可执行目录自动生成示例 `qcmd.conf`（示例为注释，默认不可用）

## 配置文件（`qcmd.conf`）

- 位置：程序可执行文件所在目录（例如 `.
bin\qcmd.conf`）
- 格式：每行 `name=command`，键值对中间使用第一个 `=` 分割
- 注释：以 `#` 或 `;` 开头的行会被忽略
- 默认生成示例（生成时示例被注释以防误执行）：

```text
# qcmd.conf 示例（默认被注释以防误执行）
# build-mesen=dotnet publish -c Release -r win-x64 -p:PublishAot=true -p:PublishSingleFile=false -p:SelfContained=true
```

程序读取配置后会将键值对装载为映射（name -> command），并在执行 `qcmd <name>` 时调用系统 shell 执行对应命令。

## 快速示例（PowerShell）

- 显示帮助（无参数也显示帮助）：

```powershell
& ".\\bin\\qcmd.exe"
```

- 列出映射：

```powershell
& ".\\bin\\qcmd.exe" list
```

- 添加或更新映射（命令可包含空格）：

```powershell
& ".\\bin\\qcmd.exe" add build-mesen=dotnet publish -c Release -r win-x64 -p:PublishAot=true -p:PublishSingleFile=false -p:SelfContained=true
# 若要避免 PowerShell 的参数拆分，也可以将整个参数用引号包起来：
& ".\\bin\\qcmd.exe" 'add build-mesen=dotnet publish -c Release -r win-x64'
```

- 执行映射：

```powershell
& ".\\bin\\qcmd.exe" build-mesen
```

- 删除映射：

```powershell
& ".\\bin\\qcmd.exe" remove build-mesen
```

## 从源码构建

- 需要：Visual Studio / MSBuild（Windows），编译器需支持 C++20（项目使用 `/std:c++20`）
- 使用 MSBuild（PowerShell 示例）：

```powershell
msbuild "D:\\GitCode\\QuickCmd(Github)\\QuickCmd.sln" /p:Configuration=Release /m
```

- 生成结果：`.\\bin\\qcmd.exe`

## 注意事项

- 命令通过系统 shell (`std::system`) 执行，执行效果受当前 shell（PowerShell / cmd）和环境变量影响
- `qcmd add` 会修改 `qcmd.conf`（覆盖写入），请在必要时备份配置
- 列表输出使用 ANSI 颜色高亮（若终端支持）

## 问题与贡献

若发现 bug 或希望提出改进，请在仓库打开 Issue 或提交 Pull Request。

## 许可

有关许可信息，请参阅仓库根目录的 `LICENSE` 文件。

---
作者/维护：仓库维护者（见仓库信息）

