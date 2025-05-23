#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import re

try:
    from bs4 import BeautifulSoup
except ImportError:
    sys.stderr.write("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")
    sys.exit(1)

def parse_problem_detail(html_content):
    soup = BeautifulSoup(html_content, 'html.parser')
    
    title_elem = soup.find('h2')
    title = title_elem.text.strip() if title_elem else "未知题目"
    
    content_div = soup.find('div', class_='problem-content')
    if not content_div:
        return "解析失败：找不到题目内容区域", "", "", "", "", ""
    
    sections = content_div.find_all('div', class_='section')
    
    description = ""
    input_desc = ""
    output_desc = ""
    sample_input = ""
    sample_output = ""
    hint = ""
    
    for section in sections:
        section_title = section.find('div', class_='section-title')
        if not section_title:
            continue
        
        section_title_text = section_title.text.strip()
        
        section_content = section.find('div', class_='section-content')
        if not section_content:
            continue
            
        content_text = section_content.text.strip()
        
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
    if len(sys.argv) == 1:
        file_path = "/tmp/never-judge/temp-problem_detail"  # 用于测试
    else:
        file_path = sys.argv[1]
    
    if not os.path.exists(file_path):
        sys.stderr.write(f"文件 {file_path} 不存在")
        sys.exit(1)
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    result = parse_problem_detail(content)
    
    print("|".join(result))

if __name__ == "__main__":
    main()
