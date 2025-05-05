#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
题目详情解析脚本

从 OpenJudge 网页中提取题目的详细信息，包括：
- 题目标题
- 题目描述
- 输入描述
- 输出描述
- 样例输入
- 样例输出
- 提示信息（如果有）

输出格式：标题|描述|输入描述|输出描述|样例输入|样例输出|提示
"""

import sys
import os
import re

try:
    from bs4 import BeautifulSoup
except ImportError:
    sys.stderr.write("请安装 BeautifulSoup4: pip install beautifulsoup4")
    sys.exit(1)

def parse_problem_detail(html_content):
    """解析题目详情"""
    soup = BeautifulSoup(html_content, 'html.parser')
    
    # 获取题目标题
    title_elem = soup.find('h2')
    title = title_elem.text.strip() if title_elem else "未知题目"
    
    # 获取题目内容区域
    content_div = soup.find('div', class_='problem-content')
    if not content_div:
        return "解析失败：找不到题目内容区域", "", "", "", "", ""
    
    # 分离各个部分
    sections = content_div.find_all('div', class_='section')
    
    description = ""
    input_desc = ""
    output_desc = ""
    sample_input = ""
    sample_output = ""
    hint = ""
    
    for section in sections:
        # 获取标题
        section_title = section.find('div', class_='section-title')
        if not section_title:
            continue
        
        section_title_text = section_title.text.strip()
        
        # 获取内容
        section_content = section.find('div', class_='section-content')
        if not section_content:
            continue
            
        content_text = section_content.text.strip()
        
        # 根据标题分类
        if "题目描述" in section_title_text:
            description = content_text
        elif "输入" in section_title_text and "样例" not in section_title_text:
            input_desc = content_text
        elif "输出" in section_title_text and "样例" not in section_title_text:
            output_desc = content_text
        elif "样例输入" in section_title_text:
            sample_input = content_text
        elif "样例输出" in section_title_text:
            sample_output = content_text
        elif "提示" in section_title_text:
            hint = content_text
    
    return title, description, input_desc, output_desc, sample_input, sample_output, hint

def main():
    # 获取输入文件路径
    if len(sys.argv) == 1:
        file_path = "/tmp/never-judge/temp-problem_detail"  # 用于测试
    else:
        file_path = sys.argv[1]
    
    # 检查文件是否存在
    if not os.path.exists(file_path):
        sys.stderr.write(f"文件 {file_path} 不存在")
        sys.exit(1)
    
    # 读取文件内容
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 解析题目详情
    result = parse_problem_detail(content)
    
    # 输出结果，使用 | 分隔各个部分
    print("|".join(result))

if __name__ == "__main__":
    main()
