import sys
try:
    from bs4 import BeautifulSoup
except ImportError:
    raise ImportError("Please install BeautifulSoup4 using 'pip install beautifulsoup4'")

def get_field(soup, name):

    inp = soup.find('input', attrs={'name': name})
    if inp and inp.has_attr('value'):
        return inp['value'].strip()

    ta = soup.find('textarea', attrs={'name': name})
    if ta:
        return ta.get_text().strip()

    sel = soup.find('select', attrs={'name': name})
    if sel:
        opt = sel.find('option', selected=True)
        if opt and opt.has_attr('value'):
            return opt['value'].strip()
    return ''

def main():
    if len(sys.argv) == 1:
        path = "/tmp/never-judge/temp-personalization"  # used for testing
    else:
        path = sys.argv[1]
    with open(path, 'r', encoding='utf-8') as f:
        html = f.read()
    soup = BeautifulSoup(html, 'html.parser')

    # 0: nickname  <- <input name="name">
    # 1: name      <- <input name="realname">
    # 2: description
    # 3: gender    （"male" or "female"）
    # 4: birthday  （format YYYY-MM-DD）
    # 5: city
    # 6: school

    fields = ['name', 'realname', 'description', 'gender', 'birthday', 'city', 'school']
    for field in fields:
        print(get_field(soup, field))
        print("======")

if __name__ == '__main__':
    main()
