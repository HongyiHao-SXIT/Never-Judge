import sys
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    sys.stderr.write("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")
    exit(1)

if len(sys.argv) == 1:
    file = "/tmp/never-judge/temp-match"  # used for testing
else:
    file = sys.argv[1]
if not os.path.exists(file):
    sys.stderr.write(f"File {file} does not exist")
    exit(1)

with open(file, 'r', encoding='utf-8') as f:
    content = f.read()

soup = BeautifulSoup(content, 'html.parser')

table = soup.find('table')
if table is None:
    sys.stderr.write("No problem table found in the file")
    exit(1)

titles = table.find('tbody').findAll('td', class_='title')

for title in titles:
    url = title.find('a')["href"]
    print(url)
