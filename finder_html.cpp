#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <set>
#include <windows.h>
#include <iomanip>

namespace fs = std::filesystem;

struct LoginRecord {
    std::string time;
    std::string location;
    std::string ip;
    std::string world;
    double x, y, z;
    std::string filePath;
    int lineNumber;
};

struct LocationInfo {
    std::string location;
    std::string ip;
    std::string world;
    double x, y, z;
    std::string time;
    std::string filePath;
    int lineNumber;
};

struct PlayerData {
    std::map<std::string, int> locationCount;
    std::map<std::string, std::vector<LocationInfo>> locationDetails;
    std::vector<LoginRecord> allRecords;
};

struct DeathData {
    int naturalDeaths = 0;   // 自然死亡 (died)
    int commandKills = 0;    // 指令杀死 (was killed)
};

struct DeathRecord {
    std::string time;
    std::string type;  // "died" or "killed"
    std::string filePath;
    int lineNumber;
};

std::string escapeHtml(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    return result;
}

std::string escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string extractTime(const std::string& line) {
    std::regex pattern(R"(\[(\d{2}:\d{2}:\d{2})\])");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();
    }
    return "";
}

std::string extractLocation(const std::string& line) {
    std::regex pattern(R"(\[PotatoIpDisplay\] Player named .+ connected from\s+(.+?)\s*$)");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        std::string location = match[1].str();
        location.erase(location.find_last_not_of(" \t\n\r") + 1);
        if (!location.empty()) {
            return location;
        }
    }
    return "";
}

std::string extractPlayerName(const std::string& line) {
    std::regex pattern(R"(Player named\s+(\w+))");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();
    }
    return "";
}

std::string extractIP(const std::string& line) {
    std::regex pattern(R"(\[/(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):)");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();
    }
    return "";
}

std::string extractWorld(const std::string& line) {
    std::regex pattern(R"(at \(\[(\w+)\])");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();
    }
    return "";
}

std::vector<double> extractCoords(const std::string& line) {
    std::regex pattern(R"(at \([^\]]+\]([^,]+),\s*([^,]+),\s*([^\)]+)\))");
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        try {
            double x = std::stod(match[1].str());
            double y = std::stod(match[2].str());
            double z = std::stod(match[3].str());
            return {x, y, z};
        } catch (...) {
            return {};
        }
    }
    return {};
}

// 新增：提取死亡消息中的玩家名
std::pair<std::string, std::string> extractDeathInfo(const std::string& line) {
    // 匹配 "玩家名 died" - 玩家名不含空格
    std::regex diedPattern(R"(\[Server thread/INFO\]:\s+(\S+)\s+died)");
    std::smatch match;
    if (std::regex_search(line, match, diedPattern)) {
        return {match[1].str(), "died"};
    }
    
    // 匹配 "玩家名 was killed" - 玩家名不含空格
    std::regex killedPattern(R"(\[Server thread/INFO\]:\s+(\S+)\s+was killed)");
    if (std::regex_search(line, match, killedPattern)) {
        return {match[1].str(), "killed"};
    }
    
    return {"", ""};
}

std::string getAbsolutePath(const fs::path& path) {
    return fs::absolute(path).string();
}

void writeHtmlHeader(std::ofstream& html) {
    html << R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>玩家登录统计分析</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Microsoft YaHei", Arial, sans-serif;
            background: linear-gradient(135deg, #b0d5f3 0%, #8ffafd 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #667eea 0%, #19e9f8 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        .header p {
            opacity: 0.9;
            font-size: 1.1em;
        }
        
        .search-bar {
            padding: 20px 30px;
            background: #f8f9fa;
            border-bottom: 2px solid #e9ecef;
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            align-items: center;
        }
        
        .search-bar input, .search-bar select {
            padding: 10px 15px;
            border: 2px solid #dee2e6;
            border-radius: 8px;
            font-size: 14px;
            transition: all 0.3s;
        }
        
        .search-bar input:focus, .search-bar select:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .search-bar input[type="text"] {
            flex: 1;
            min-width: 250px;
        }
        
        .search-bar button {
            padding: 10px 25px;
            background: #ff8800;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s;
        }
        
        .search-bar button:hover {
            background: #ffbb00;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.3);
        }
        
        .tabs {
            display: flex;
            background: #f8f9fa;
            border-bottom: 2px solid #dee2e6;
            overflow-x: auto;
        }
        
        .tab {
            padding: 15px 30px;
            cursor: pointer;
            border: none;
            background: none;
            font-size: 15px;
            font-weight: 600;
            color: #6c757d;
            transition: all 0.3s;
            white-space: nowrap;
            position: relative;
        }
        
        .tab:hover {
            color: #ff9900;
            background: rgba(102, 126, 234, 0.05);
        }
        
        .tab.active {
            color: #0099ff;
            background: white;
        }
        
        .tab.active::after {
            content: '';
            position: absolute;
            bottom: -2px;
            left: 0;
            right: 0;
            height: 3px;
            background: #ffa600;
        }
        
        .tab-content {
            display: none;
            padding: 30px;
            animation: fadeIn 0.3s;
        }
        
        .tab-content.active {
            display: block;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .stat-card {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 25px;
            border-radius: 12px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        
        .stat-card h3 {
            font-size: 0.9em;
            opacity: 0.9;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .stat-card .value {
            font-size: 2.5em;
            font-weight: bold;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
            background: white;
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 2px 8px rgba(0,0,0,0.05);
        }
        
        thead {
            background: linear-gradient(135deg, #3a3d34 0%, #585444 100%);
            color: white;
        }
        
        th {
            padding: 15px;
            text-align: left;
            font-weight: 600;
            text-transform: uppercase;
            font-size: 0.85em;
            letter-spacing: 0.5px;
        }
        
        td {
            padding: 12px 15px;
            border-bottom: 1px solid #f1f3f5;
        }
        
        tbody tr {
            transition: all 0.2s;
        }
        
        tbody tr:hover {
            background: #f8f9fa;
            transform: scale(1.01);
            box-shadow: 0 2px 8px rgba(0,0,0,0.05);
        }
        
        .rank-badge {
            display: inline-block;
            width: 30px;
            height: 30px;
            line-height: 30px;
            text-align: center;
            border-radius: 50%;
            font-weight: bold;
            color: white;
        }
        
        .rank-1 { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); }
        .rank-2 { background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%); }
        .rank-3 { background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%); }
        .rank-other { background: linear-gradient(135deg, #fa709a 0%, #fee140 100%); }
        
        .details-row {
            display: none;
            background: #f8f9fa !important;
        }
        
        .details-row td {
            padding: 20px;
        }
        
        .details-content {
            background: white;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        
        .details-table {
            margin-top: 10px;
            font-size: 0.9em;
        }
        
        .toggle-btn {
            cursor: pointer;
            color: #667eea;
            font-weight: 600;
            transition: all 0.3s;
        }
        
        .toggle-btn:hover {
            color: #5568d3;
            text-decoration: underline;
        }
        
        .badge {
            display: inline-block;
            padding: 4px 10px;
            border-radius: 12px;
            font-size: 0.85em;
            font-weight: 600;
        }
        
        .badge-primary { background: #e7f5ff; color: #1c7ed6; }
        .badge-success { background: #d3f9d8; color: #2b8a3e; }
        .badge-warning { background: #fff3bf; color: #e67700; }
        .badge-danger { background: #ffe3e3; color: #c92a2a; }
        
        .no-results {
            text-align: center;
            padding: 40px;
            color: #6c757d;
            font-size: 1.1em;
        }
        
        .file-link {
            color: #6c757d;
            font-size: 0.85em;
            text-decoration: none;
        }
        
        .file-link:hover {
            color: #667eea;
            text-decoration: underline;
        }
        
        .coord {
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            color: #495057;
        }
        
        @media (max-width: 768px) {
            .header h1 { font-size: 1.8em; }
            .search-bar { flex-direction: column; }
            .search-bar input, .search-bar select { width: 100%; }
            table { font-size: 0.85em; }
            th, td { padding: 8px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎮 玩家登录统计分析系统</h1>
            <p>Minecraft Server Login Analytics Dashboard</p>
        </div>
        
        <div class="search-bar">
            <input type="text" id="searchInput" placeholder="🔍 搜索玩家、IP、地点..." onkeyup="filterData()">
            <select id="filterType" onchange="filterData()">
                <option value="all">全部</option>
                <option value="player">玩家</option>
                <option value="location">地点</option>
                <option value="ip">IP</option>
            </select>
            <button onclick="resetFilter()">重置</button>
            <button onclick="exportData()">导出数据</button>
        </div>
        
        <div class="tabs">
            <button class="tab active" onclick="switchTab(0)">📊 概览统计</button>
            <button class="tab" onclick="switchTab(1)">🌍 地点统计</button>
            <button class="tab" onclick="switchTab(2)">👤 玩家统计</button>
            <button class="tab" onclick="switchTab(3)">💀 死亡统计</button>
            <button class="tab" onclick="switchTab(4)">📈 多维分析</button>
            <button class="tab" onclick="switchTab(5)">📋 完整记录</button>
        </div>
)HTML";
}

void writeHtmlFooter(std::ofstream& html) {
    html << R"(
    </div>
    
    <script>
        function switchTab(index) {
            const tabs = document.querySelectorAll('.tab');
            const contents = document.querySelectorAll('.tab-content');
            
            tabs.forEach((tab, i) => {
                if (i === index) {
                    tab.classList.add('active');
                    contents[i].classList.add('active');
                } else {
                    tab.classList.remove('active');
                    contents[i].classList.remove('active');
                }
            });
        }
        
        function toggleDetails(id) {
            const row = document.getElementById(id);
            row.style.display = row.style.display === 'none' ? 'table-row' : 'none';
        }
        
        function filterData() {
            const input = document.getElementById('searchInput').value.toLowerCase();
            const filterType = document.getElementById('filterType').value;
            const allRows = document.querySelectorAll('tbody tr:not(.details-row)');
            
            allRows.forEach(row => {
                const text = row.textContent.toLowerCase();
                const shouldShow = text.includes(input);
                row.style.display = shouldShow ? '' : 'none';
                
                // Hide associated details row
                const detailsRow = row.nextElementSibling;
                if (detailsRow && detailsRow.classList.contains('details-row')) {
                    detailsRow.style.display = 'none';
                }
            });
        }
        
        function resetFilter() {
            document.getElementById('searchInput').value = '';
            document.getElementById('filterType').value = 'all';
            filterData();
        }
        
        function exportData() {
            const data = window.rawData;
            const json = JSON.stringify(data, null, 2);
            const blob = new Blob([json], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'login_data.json';
            a.click();
        }
        
        // Initialize
        document.addEventListener('DOMContentLoaded', function() {
            console.log('统计系统已加载');
        });
    </script>
</body>
</html>
)";
}

int main() {
    SetConsoleOutputCP(65001);
    
    std::map<std::string, PlayerData> playerData;
    std::map<std::string, std::set<std::string>> playerIPs;
    std::map<std::pair<std::string, std::string>, std::vector<LocationInfo>> locationIPDetails;
    std::map<std::string, DeathData> playerDeaths;  // 新增：死亡统计
    std::map<std::string, std::vector<DeathRecord>> playerDeathRecords;  // 新增：死亡详细记录
    
    int filesProcessed = 0;
    int totalLines = 0;
    int matchedLines = 0;
    int totalDeaths = 0;  // 新增：死亡总数
    
    std::cout << "开始扫描日志文件..." << std::endl;
    
    if (!fs::exists("1")) {
        std::cerr << "错误: 目录 '1' 不存在" << std::endl;
        return 1;
    }
    
    std::vector<fs::path> logFiles;
    for (const auto& entry : fs::directory_iterator("1")) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            logFiles.push_back(entry.path());
        }
    }
    
    std::cout << "找到 " << logFiles.size() << " 个日志文件" << std::endl << std::endl;
    
    std::string currentPlayer;
    std::string currentLocation;
    std::string currentIP;
    std::string currentTime;
    std::string currentFilePath;
    int currentLineNumber;
    
    for (const auto& logFile : logFiles) {
        std::ifstream file(logFile);
        if (!file.is_open()) {
            std::cerr << "无法打开: " << logFile.filename() << std::endl;
            continue;
        }
        
        std::string line;
        int fileLines = 0;
        int fileMatches = 0;
        int lineNumber = 0;
        std::string absolutePath = getAbsolutePath(logFile);
        
        while (std::getline(file, line)) {
            fileLines++;
            lineNumber++;
            totalLines++;
            
            // 新增：检测死亡消息
            auto [deathPlayer, deathType] = extractDeathInfo(line);
            if (!deathPlayer.empty() && !deathType.empty()) {
                std::string time = extractTime(line);
                
                if (deathType == "died") {
                    playerDeaths[deathPlayer].naturalDeaths++;
                } else if (deathType == "killed") {
                    playerDeaths[deathPlayer].commandKills++;
                }
                
                DeathRecord deathRec;
                deathRec.time = time;
                deathRec.type = deathType;
                deathRec.filePath = absolutePath;
                deathRec.lineNumber = lineNumber;
                playerDeathRecords[deathPlayer].push_back(deathRec);
                
                totalDeaths++;
                
                std::cout << "💀 时间: " << time << " | 玩家: " << deathPlayer 
                          << " | 类型: " << (deathType == "died" ? "自然死亡" : "指令杀死") << std::endl;
                std::cout << "  └─ 文件: " << absolutePath << ":" << lineNumber << std::endl;
            }
            
            if (line.find("[PotatoIpDisplay] Player named") != std::string::npos && 
                line.find("connected from") != std::string::npos) {
                
                std::string player = extractPlayerName(line);
                std::string location = extractLocation(line);
                std::string time = extractTime(line);
                
                if (!player.empty() && !location.empty()) {
                    currentPlayer = player;
                    currentLocation = location;
                    currentIP = "";
                    currentTime = time;
                    currentFilePath = absolutePath;
                    currentLineNumber = lineNumber;
                    matchedLines++;
                    fileMatches++;
                    
                    std::cout << "✓ 时间: " << time << " | 玩家: " << player 
                              << " | 位置: " << location << std::endl;
                    std::cout << "  └─ 文件: " << absolutePath << ":" << lineNumber << std::endl;
                }
            }
            else if (!currentPlayer.empty() && line.find("logged in with entity id") != std::string::npos &&
                     line.find("[/") != std::string::npos) {
                
                std::string ip = extractIP(line);
                if (!ip.empty()) {
                    currentIP = ip;
                    std::string world = extractWorld(line);
                    auto coords = extractCoords(line);
                    
                    if (!currentLocation.empty()) {
                        playerData[currentPlayer].locationCount[currentLocation]++;
                        playerIPs[currentPlayer].insert(ip);
                        
                        LocationInfo info;
                        info.location = currentLocation;
                        info.ip = ip;
                        info.world = world;
                        info.time = currentTime;
                        info.filePath = currentFilePath;
                        info.lineNumber = currentLineNumber;
                        if (coords.size() == 3) {
                            info.x = coords[0];
                            info.y = coords[1];
                            info.z = coords[2];
                        } else {
                            info.x = 0;
                            info.y = 0;
                            info.z = 0;
                        }
                        
                        playerData[currentPlayer].locationDetails[currentLocation].push_back(info);
                        locationIPDetails[{currentLocation, ip}].push_back(info);
                        
                        LoginRecord record;
                        record.time = currentTime;
                        record.location = currentLocation;
                        record.ip = ip;
                        record.world = world;
                        record.x = info.x;
                        record.y = info.y;
                        record.z = info.z;
                        record.filePath = currentFilePath;
                        record.lineNumber = currentLineNumber;
                        playerData[currentPlayer].allRecords.push_back(record);
                        
                        std::cout << "  └─ IP: " << ip << " | 世界: " << world;
                        if (coords.size() == 3) {
                            std::cout << " | 坐标: (" << coords[0] << ", " << coords[1] << ", " << coords[2] << ")";
                        }
                        std::cout << std::endl;
                        
                        matchedLines++;
                        fileMatches++;
                    }
                    
                    currentPlayer = "";
                }
            }
        }
        
        file.close();
        filesProcessed++;
        std::cout << "✓ 已处理: " << logFile.filename() << " (" << fileLines << " 行, " 
                  << fileMatches << " 条记录)" << std::endl;
    }
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "处理完成: " << filesProcessed << " 个文件, " 
              << totalLines << " 行, " << matchedLines << " 条有效记录" << std::endl;
    std::cout << "死亡记录: " << totalDeaths << " 条" << std::endl;
    std::cout << std::string(50, '=') << "\n" << std::endl;
    
    // Generate HTML
    std::ofstream html("result.html");
    if (!html.is_open()) {
        std::cerr << "无法创建 result.html" << std::endl;
        return 1;
    }
    
    writeHtmlHeader(html);
    
    // Tab 0: Overview Statistics
    html << R"(
        <div class="tab-content active">
            <h2>📊 概览统计</h2>
            <div class="stats-grid">
)";
    
    int totalPlayers = playerData.size();
    int totalRecords = matchedLines;
    int totalLocations = 0;
    std::set<std::string> allIPs;
    
    for (const auto& [player, ips] : playerIPs) {
        allIPs.insert(ips.begin(), ips.end());
    }
    
    for (const auto& entry : locationIPDetails) {
        totalLocations++;
    }
    
    html << "<div class=\"stat-card\"><h3>总玩家数</h3><div class=\"value\">" << totalPlayers << "</div></div>\n";
    html << "<div class=\"stat-card\"><h3>总登录记录</h3><div class=\"value\">" << totalRecords << "</div></div>\n";
    html << "<div class=\"stat-card\"><h3>不同地点</h3><div class=\"value\">" << totalLocations << "</div></div>\n";
    html << "<div class=\"stat-card\"><h3>不同IP</h3><div class=\"value\">" << allIPs.size() << "</div></div>\n";
    html << "<div class=\"stat-card\"><h3>总死亡次数</h3><div class=\"value\">" << totalDeaths << "</div></div>\n";
    
    html << R"(
            </div>
            
            <h3 style="margin-top: 30px; margin-bottom: 15px;">🏆 TOP 10 玩家（按登录次数）</h3>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>玩家</th>
                        <th>登录次数</th>
                        <th>不同地点</th>
                        <th>不同IP</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    std::vector<std::tuple<std::string, int, int, int>> playerStats;
    for (const auto& [player, data] : playerData) {
        int recordCount = 0;
        for (const auto& [location, count] : data.locationCount) {
            recordCount += count;
        }
        playerStats.push_back({player, recordCount, data.locationCount.size(), playerIPs[player].size()});
    }
    
    std::sort(playerStats.begin(), playerStats.end(),
              [](const auto& a, const auto& b) { return std::get<1>(a) > std::get<1>(b); });
    
    for (int i = 0; i < std::min(10, (int)playerStats.size()); ++i) {
        const auto& [player, recordCount, locCount, ipCount] = playerStats[i];
        std::string rankClass = i < 3 ? "rank-" + std::to_string(i + 1) : "rank-other";
        
        html << "<tr><td><span class=\"rank-badge " << rankClass << "\">" << (i + 1) << "</span></td>";
        html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
        html << "<td>" << recordCount << "</td>";
        html << "<td><span class=\"badge badge-primary\">" << locCount << "</span></td>";
        html << "<td><span class=\"badge badge-success\">" << ipCount << "</span></td></tr>\n";
    }
    
    html << "</tbody></table>\n";
    
    // 新增：TOP 10 自然死亡玩家
    html << R"(
            <h3 style="margin-top: 30px; margin-bottom: 15px;">💀 TOP 10 玩家（按自然死亡次数）</h3>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>玩家</th>
                        <th>自然死亡</th>
                        <th>指令杀死</th>
                        <th>总死亡</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    std::vector<std::tuple<std::string, int, int, int>> deathStats;
    for (const auto& [player, deaths] : playerDeaths) {
        deathStats.push_back({player, deaths.naturalDeaths, deaths.commandKills, 
                              deaths.naturalDeaths + deaths.commandKills});
    }
    
    std::sort(deathStats.begin(), deathStats.end(),
              [](const auto& a, const auto& b) { return std::get<1>(a) > std::get<1>(b); });
    
    for (int i = 0; i < std::min(10, (int)deathStats.size()); ++i) {
        const auto& [player, naturalDeaths, commandKills, totalDeath] = deathStats[i];
        std::string rankClass = i < 3 ? "rank-" + std::to_string(i + 1) : "rank-other";
        
        html << "<tr><td><span class=\"rank-badge " << rankClass << "\">" << (i + 1) << "</span></td>";
        html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
        html << "<td><span class=\"badge badge-danger\">" << naturalDeaths << "</span></td>";
        html << "<td><span class=\"badge badge-warning\">" << commandKills << "</span></td>";
        html << "<td>" << totalDeath << "</td></tr>\n";
    }
    
    html << "</tbody></table></div>\n";
    
    // Tab 1: Location Statistics
    html << R"(
        <div class="tab-content">
            <h2>🌍 地点统计</h2>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>地点</th>
                        <th>登录次数</th>
                        <th>操作</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    std::vector<std::pair<std::string, int>> locationRanking;
    for (const auto& entry : locationIPDetails) {
        locationRanking.push_back({entry.first.first, entry.second.size()});
    }
    std::sort(locationRanking.begin(), locationRanking.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (int i = 0; i < locationRanking.size(); ++i) {
        const auto& location = locationRanking[i].first;
        int count = locationRanking[i].second;
        std::string detailId = "loc-details-" + std::to_string(i);
        
        html << "<tr><td>" << (i + 1) << "</td>";
        html << "<td><strong>" << escapeHtml(location) << "</strong></td>";
        html << "<td>" << count << "</td>";
        html << "<td><span class=\"toggle-btn\" onclick=\"toggleDetails('" << detailId << "')\">📋 查看详情</span></td></tr>\n";
        
        html << "<tr id=\"" << detailId << "\" class=\"details-row\"><td colspan=\"4\">";
        html << "<div class=\"details-content\">";
        html << "<h4>详细记录</h4>";
        html << "<table class=\"details-table\"><thead><tr><th>时间</th><th>玩家</th><th>IP</th><th>世界</th><th>坐标</th><th>文件:行</th></tr></thead><tbody>";
        
        for (const auto& [locIP, details] : locationIPDetails) {
            if (locIP.first == location) {
                for (const auto& detail : details) {
                    std::string playerName;
                    for (const auto& [player, data] : playerData) {
                        if (data.locationDetails.count(location)) {
                            playerName = player;
                            break;
                        }
                    }
                    
                    html << "<tr><td>" << escapeHtml(detail.time) << "</td>";
                    html << "<td>" << escapeHtml(playerName) << "</td>";
                    html << "<td><code>" << escapeHtml(detail.ip) << "</code></td>";
                    html << "<td>" << (detail.world.empty() ? "-" : escapeHtml(detail.world)) << "</td>";
                    
                    if (detail.x != 0 || detail.y != 0 || detail.z != 0) {
                        html << "<td class=\"coord\">" << std::fixed << std::setprecision(1) 
                             << detail.x << ", " << detail.y << ", " << detail.z << "</td>";
                    } else {
                        html << "<td>-</td>";
                    }
                    
                    std::string filename = fs::path(detail.filePath).filename().string();
                    html << "<td><a class=\"file-link\" title=\"" << escapeHtml(detail.filePath) << "\">" 
                         << escapeHtml(filename) << ":" << detail.lineNumber << "</a></td></tr>\n";
                }
            }
        }
        
        html << "</tbody></table></div></td></tr>\n";
    }
    
    html << "</tbody></table></div>\n";
    
    // Tab 2: Player Statistics
    html << R"(
        <div class="tab-content">
            <h2>👤 玩家统计</h2>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>玩家</th>
                        <th>总登录</th>
                        <th>不同地点</th>
                        <th>不同IP</th>
                        <th>操作</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    for (int i = 0; i < playerStats.size(); ++i) {
        const auto& [player, recordCount, locCount, ipCount] = playerStats[i];
        std::string detailId = "player-details-" + std::to_string(i);
        
        html << "<tr><td>" << (i + 1) << "</td>";
        html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
        html << "<td>" << recordCount << "</td>";
        html << "<td><span class=\"badge badge-primary\">" << locCount << "</span></td>";
        html << "<td><span class=\"badge badge-success\">" << ipCount << "</span></td>";
        html << "<td><span class=\"toggle-btn\" onclick=\"toggleDetails('" << detailId << "')\">📋 查看详情</span></td></tr>\n";
        
        html << "<tr id=\"" << detailId << "\" class=\"details-row\"><td colspan=\"6\">";
        html << "<div class=\"details-content\">";
        html << "<h4>详细记录</h4>";
        html << "<table class=\"details-table\"><thead><tr><th>时间</th><th>地点</th><th>IP</th><th>世界</th><th>坐标</th><th>文件:行</th></tr></thead><tbody>";
        
        for (const auto& record : playerData[player].allRecords) {
            html << "<tr><td>" << escapeHtml(record.time) << "</td>";
            html << "<td>" << escapeHtml(record.location) << "</td>";
            html << "<td><code>" << escapeHtml(record.ip) << "</code></td>";
            html << "<td>" << (record.world.empty() ? "-" : escapeHtml(record.world)) << "</td>";
            
            if (record.x != 0 || record.y != 0 || record.z != 0) {
                html << "<td class=\"coord\">" << std::fixed << std::setprecision(1) 
                     << record.x << ", " << record.y << ", " << record.z << "</td>";
            } else {
                html << "<td>-</td>";
            }
            
            std::string filename = fs::path(record.filePath).filename().string();
            html << "<td><a class=\"file-link\" title=\"" << escapeHtml(record.filePath) << "\">" 
                 << escapeHtml(filename) << ":" << record.lineNumber << "</a></td></tr>\n";
        }
        
        html << "</tbody></table></div></td></tr>\n";
    }
    
    html << "</tbody></table></div>\n";
    
    // Tab 3: Death Statistics (新增)
    html << R"(
        <div class="tab-content">
            <h2>💀 死亡统计</h2>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>玩家</th>
                        <th>自然死亡</th>
                        <th>指令杀死</th>
                        <th>总死亡</th>
                        <th>操作</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    // 按总死亡次数排序
    std::vector<std::tuple<std::string, int, int, int>> allDeathStats;
    for (const auto& [player, deaths] : playerDeaths) {
        allDeathStats.push_back({player, deaths.naturalDeaths, deaths.commandKills, 
                                 deaths.naturalDeaths + deaths.commandKills});
    }
    
    std::sort(allDeathStats.begin(), allDeathStats.end(),
              [](const auto& a, const auto& b) { return std::get<3>(a) > std::get<3>(b); });
    
    for (int i = 0; i < allDeathStats.size(); ++i) {
        const auto& [player, naturalDeaths, commandKills, totalDeath] = allDeathStats[i];
        std::string detailId = "death-details-" + std::to_string(i);
        
        html << "<tr><td>" << (i + 1) << "</td>";
        html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
        html << "<td><span class=\"badge badge-danger\">" << naturalDeaths << "</span></td>";
        html << "<td><span class=\"badge badge-warning\">" << commandKills << "</span></td>";
        html << "<td>" << totalDeath << "</td>";
        html << "<td><span class=\"toggle-btn\" onclick=\"toggleDetails('" << detailId << "')\">📋 查看详情</span></td></tr>\n";
        
        html << "<tr id=\"" << detailId << "\" class=\"details-row\"><td colspan=\"6\">";
        html << "<div class=\"details-content\">";
        html << "<h4>详细记录</h4>";
        html << "<table class=\"details-table\"><thead><tr><th>时间</th><th>类型</th><th>文件:行</th></tr></thead><tbody>";
        
        if (playerDeathRecords.count(player)) {
            for (const auto& record : playerDeathRecords[player]) {
                html << "<tr><td>" << escapeHtml(record.time) << "</td>";
                html << "<td>";
                if (record.type == "died") {
                    html << "<span class=\"badge badge-danger\">自然死亡</span>";
                } else {
                    html << "<span class=\"badge badge-warning\">指令杀死</span>";
                }
                html << "</td>";
                
                std::string filename = fs::path(record.filePath).filename().string();
                html << "<td><a class=\"file-link\" title=\"" << escapeHtml(record.filePath) << "\">" 
                     << escapeHtml(filename) << ":" << record.lineNumber << "</a></td></tr>\n";
            }
        }
        
        html << "</tbody></table></div></td></tr>\n";
    }
    
    html << "</tbody></table></div>\n";
    
    // Tab 4: Multi-dimensional Analysis
    html << R"(
        <div class="tab-content">
            <h2>📈 多维度分析</h2>
            <p>按不同地点数量排序</p>
            <table>
                <thead>
                    <tr>
                        <th>排名</th>
                        <th>玩家</th>
                        <th>不同地点数</th>
                        <th>不同IP数</th>
                        <th>总记录数</th>
                        <th>最常登录地点</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    std::vector<std::tuple<std::string, int, int, int>> multiStats = playerStats;
    std::sort(multiStats.begin(), multiStats.end(),
              [](const auto& a, const auto& b) { return std::get<2>(a) > std::get<2>(b); });
    
    for (int i = 0; i < multiStats.size(); ++i) {
        const auto& [player, recordCount, locCount, ipCount] = multiStats[i];
        
        auto maxLocationIt = std::max_element(
            playerData[player].locationCount.begin(),
            playerData[player].locationCount.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        
        html << "<tr><td>" << (i + 1) << "</td>";
        html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
        html << "<td><span class=\"badge badge-warning\">" << locCount << "</span></td>";
        html << "<td><span class=\"badge badge-success\">" << ipCount << "</span></td>";
        html << "<td>" << recordCount << "</td>";
        
        if (maxLocationIt != playerData[player].locationCount.end()) {
            html << "<td>" << escapeHtml(maxLocationIt->first) << " <span class=\"badge badge-danger\">" 
                 << maxLocationIt->second << "次</span></td>";
        } else {
            html << "<td>-</td>";
        }
        
        html << "</tr>\n";
    }
    
    html << "</tbody></table></div>\n";
    
    // Tab 5: Complete Records
    html << R"(
        <div class="tab-content">
            <h2>📋 完整记录</h2>
            <p>所有登录记录的完整列表</p>
            <table>
                <thead>
                    <tr>
                        <th>#</th>
                        <th>时间</th>
                        <th>玩家</th>
                        <th>地点</th>
                        <th>IP</th>
                        <th>世界</th>
                        <th>坐标</th>
                        <th>文件:行</th>
                    </tr>
                </thead>
                <tbody>
)";
    
    int recordIndex = 1;
    for (const auto& [player, data] : playerData) {
        for (const auto& record : data.allRecords) {
            html << "<tr><td>" << recordIndex++ << "</td>";
            html << "<td>" << escapeHtml(record.time) << "</td>";
            html << "<td><strong>" << escapeHtml(player) << "</strong></td>";
            html << "<td>" << escapeHtml(record.location) << "</td>";
            html << "<td><code>" << escapeHtml(record.ip) << "</code></td>";
            html << "<td>" << (record.world.empty() ? "-" : escapeHtml(record.world)) << "</td>";
            
            if (record.x != 0 || record.y != 0 || record.z != 0) {
                html << "<td class=\"coord\">" << std::fixed << std::setprecision(1) 
                     << record.x << ", " << record.y << ", " << record.z << "</td>";
            } else {
                html << "<td>-</td>";
            }
            
            std::string filename = fs::path(record.filePath).filename().string();
            html << "<td><a class=\"file-link\" title=\"" << escapeHtml(record.filePath) << "\">" 
                 << escapeHtml(filename) << ":" << record.lineNumber << "</a></td></tr>\n";
        }
    }
    
    html << "</tbody></table></div>\n";
    
    // Add raw data for export
    html << "<script>\nwindow.rawData = {\n";
    html << "  players: [\n";
    bool firstPlayer = true;
    for (const auto& [player, data] : playerData) {
        if (!firstPlayer) html << ",\n";
        firstPlayer = false;
        
        html << "    {\n      name: \"" << escapeJson(player) << "\",\n      records: [\n";
        bool firstRecord = true;
        for (const auto& record : data.allRecords) {
            if (!firstRecord) html << ",\n";
            firstRecord = false;
            
            html << "        {time: \"" << escapeJson(record.time) << "\", "
                 << "location: \"" << escapeJson(record.location) << "\", "
                 << "ip: \"" << escapeJson(record.ip) << "\", "
                 << "world: \"" << escapeJson(record.world) << "\"}";
        }
        html << "\n      ]\n    }";
    }
    html << "\n  ],\n";
    
    // Add death data to export
    html << "  deaths: [\n";
    bool firstDeath = true;
    for (const auto& [player, deaths] : playerDeaths) {
        if (!firstDeath) html << ",\n";
        firstDeath = false;
        
        html << "    {name: \"" << escapeJson(player) << "\", "
             << "naturalDeaths: " << deaths.naturalDeaths << ", "
             << "commandKills: " << deaths.commandKills << "}";
    }
    html << "\n  ]\n};\n</script>\n";
    
    writeHtmlFooter(html);
    html.close();
    
    std::cout << "✓ 结果已保存到 result.html" << std::endl;
    std::cout << std::endl << "统计摘要:" << std::endl;
    std::cout << "  - 总玩家数: " << totalPlayers << std::endl;
    std::cout << "  - 总登录记录: " << totalRecords << std::endl;
    std::cout << "  - 不同地点数: " << totalLocations << std::endl;
    std::cout << "  - 不同IP数: " << allIPs.size() << std::endl;
    std::cout << "  - 总死亡次数: " << totalDeaths << std::endl;
    
    return 0;
}