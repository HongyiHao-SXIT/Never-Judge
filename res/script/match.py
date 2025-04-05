import sys
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    raise ImportError("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")

if len(sys.argv) == 1:
    file = "/tmp/never-judge/temp-match"  # used for testing
else:
    file = sys.argv[1]
if not os.path.exists(file):
    raise Exception(f"File {file} does not exist")

with open(file, 'r', encoding='utf-8') as f:
    content = f.read()

soup = BeautifulSoup(content, 'html.parser')

table = soup.find('table')
if table is None:
    raise Exception("No problem table found in the file")

titles = table.find('tbody').findAll('td', class_='title')

for title in titles:
    url = title.find('a')["href"]
    print(url)
