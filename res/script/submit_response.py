import sys
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    raise ImportError("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")

if len(sys.argv) == 1:
    file = "/tmp/never-judge/temp-submit_result" # used for testing
else:
    file = sys.argv[1]
if not os.path.exists(file):
    sys.stderr.write(f"File {file} does not exist")
    exit(1)

with open(file, 'r', encoding='utf-8') as f:
    content = f.read()

soup = BeautifulSoup(content, 'html.parser')
status = soup.find('div', class_="submitStatus")
if status is None:
    sys.stderr.write("No compile status found!")
    exit(1)

print(status.find('p', class_='compile-status').find('a').text)
messages = status.find_all('pre')
for message in messages:
    # do not print <pre class="sh_sourceCode">
    if message is not None and not message.has_attr("class"):
        print(message.text)
