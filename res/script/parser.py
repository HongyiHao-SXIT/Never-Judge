import sys
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    raise ImportError("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")

if len(sys.argv) == 1:
    file = "/tmp/never-judge/temp-problem"  # used for testing
else:
    file = sys.argv[1]
if not os.path.exists(file):
    sys.stderr.write(f"File {file} does not exist")
    exit(1)

with open(file, 'r', encoding='utf-8') as f:
    content = f.read()

soup = BeautifulSoup(content, 'html.parser')

title = soup.find('div', id='pageTitle').find('h2')
if title is None:
    sys.stderr.write("No title found in the file")
    exit(1)

title = title.text.strip()

content = soup.find('dl', class_='problem-content')
if content is None:
    sys.stderr.write("No problem content found in the file")
    exit(1)

CSS = """<style>
dt { font-size: 20px; font-weight: bold; margin: 5px; }
pre { background-color: #222222;}
</style>
"""

html = CSS + str(content)

print(title)
print(html)
