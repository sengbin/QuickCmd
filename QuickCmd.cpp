// QuickCmd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cctype>

using std::string;
using std::map;

/**
 * @brief 去除字符串左右空白
 * @param s 输入字符串
 * @return 去除空白后的字符串
 */
static string trim(const string &s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    if (start == s.size()) return string();
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) end--;
    return s.substr(start, end - start + 1);
}

/**
 * @brief 获取可执行文件的所在目录（基于 argv[0]）
 * @param argv0 main 的第一个参数
 * @return 可执行文件所在目录（若无法解析，返回当前工作目录）
 *
 * 说明：为避免依赖 Windows.h，本函数优先解析 argv[0] 中的路径部分。
 * 若 argv[0] 不包含路径分隔符，则使用当前工作目录作为可执行目录的近似值。
 */
static std::filesystem::path getExecutableDir(const char *argv0)
{
    if (!argv0) return std::filesystem::current_path();
    string s(argv0);
    // 查找最后一个路径分隔符（兼容 / 和 \\）
    size_t pos = s.find_last_of("\\/");
    if (pos != string::npos) {
        string dir = s.substr(0, pos);
        std::filesystem::path p(dir);
        if (p.is_absolute()) return p;
        try {
            return std::filesystem::current_path() / p;
        } catch (...) {
            return std::filesystem::current_path();
        }
    }
    // 未包含路径，退回到当前工作目录
    try {
        return std::filesystem::current_path();
    } catch (...) {
        return std::filesystem::path(".");
    }
}

/**
 * @brief 从配置文件读取映射（格式：name=command，每行一条）
 * @param path 配置文件完整路径
 * @return 名称到命令的映射（未找到文件返回空映射）
 */
static map<string, string> loadConfig(const std::filesystem::path &path)
{
    map<string, string> result;
    std::ifstream ifs(path);
    if (!ifs.is_open()) return result;
    string line;
    while (std::getline(ifs, line)) {
        string t = trim(line);
        if (t.empty()) continue;
        if (t[0] == '#' || t[0] == ';') continue; // 注释行
        size_t eq = t.find('=');
        if (eq == string::npos) continue;
        string key = trim(t.substr(0, eq));
        string val = trim(t.substr(eq + 1));
        if (!key.empty()) result[key] = val;
    }
    return result;
}

/**
 * @brief 在指定路径创建示例配置文件
 * @param path 要创建的配置文件路径
 * @return 创建成功返回 true，否则返回 false
 */
static bool createDefaultConfig(const std::filesystem::path &path)
{
    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;
    ofs << "# qcmd 配置文件示例\n";
    ofs << "# 格式： name=command\n";
    ofs << "# 说明：在命令行中运行 qcmd <name> 会执行对应的 command。\n";
    ofs << "# 注：以 '#' 或 ';' 开头的行为注释，会被忽略。\n";
    ofs << "# 示例：\n";
    ofs << "#   在命令行输入：qcmd command\n";
    ofs << "#   将执行：cmd（打开命令行）\n";
    // 默认示例映射使用注释，用户需要取消注释以启用
    ofs << "# command=cmd  # 示例：默认被注释，取消注释以启用\n";
    ofs.close();
    return true;
}

int main(int argc, char **argv)
{
    // 获取程序所在目录并定位配置文件 qcmd.conf
    std::filesystem::path exeDir = getExecutableDir(argv && argv[0] ? argv[0] : nullptr);
    std::filesystem::path cfgPath = exeDir / "qcmd.conf";

    // 若配置文件不存在，则自动生成一个示例配置
    if (!std::filesystem::exists(cfgPath)) {
        bool ok = createDefaultConfig(cfgPath);
        if (ok) {
            // 将提示文字改为“生成配置”（不显示“示例”）
            std::cout << "未找到配置文件，已在程序目录生成配置：" << cfgPath.string() << std::endl;
        } else {
            std::cerr << "未找到配置文件，尝试生成配置失败：" << cfgPath.string() << std::endl;
        }
    }

    // 加载配置映射
    map<string, string> mappings = loadConfig(cfgPath);

    // 无参数：列出所有映射
    if (argc <= 1) {
        if (mappings.empty()) {
            std::cout << "配置文件为空或未包含任何映射（文件位置：" << cfgPath.string() << "）。" << std::endl;
            return 0;
        }
        std::cout << "命令映射列表：" << std::endl;
        // 使用 ANSI 控制序列为左侧名称上色（绿色），然后以空格分隔展示命令
        const string color_start = "\x1b[32m"; // 绿色
        const string color_end = "\x1b[0m";    // 重置颜色
        for (const auto &kv : mappings) {
            const string &key = kv.first;
            const string &val = kv.second;
            std::cout << color_start << key << color_end << " " << val << std::endl;
        }
        return 0;
    }

    // 带参数：查找并执行对应映射
    string name = argv[1];
    auto it = mappings.find(name);
    if (it == mappings.end()) {
        std::cerr << "未找到命令映射：" << name << std::endl;
        return 2;
    }

    const string &command = it->second;
    std::cout << "执行映射 '" << name << "' -> " << command << std::endl;

    // 使用 system 调用执行命令，并返回命令的退出码
    int rc = std::system(command.c_str());
    std::cout << "命令退出码: " << rc << std::endl;
    return rc;
}
