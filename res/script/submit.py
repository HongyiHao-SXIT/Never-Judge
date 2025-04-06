import sys
import os

try:
    from bs4 import BeautifulSoup
except ImportError:
    raise ImportError("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")

if len(sys.argv) == 1:
    file = "/tmp/never-judge/temp-submit"  # used for testing
else:
    file = sys.argv[1]
if not os.path.exists(file):
    raise Exception(f"File {file} does not exist")

with open(file, 'r', encoding='utf-8') as f:
    content = f.read()

soup = BeautifulSoup(content, 'html.parser')
submit = soup.find('input', {'name': 'contestId'})
contest = submit['value']
problem = soup.find('input', {'name': 'problemNumber'})["value"]
print(contest)
print(problem)

for language in soup.find_all('input', {'name': 'language'}):
    print(language['value'], language.next_sibling.strip())

