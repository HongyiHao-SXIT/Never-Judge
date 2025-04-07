import sys
import argparse

try:
    import requests
except ImportError:
    sys.stderr.write("Please install requests module: pip install requests")
    exit(1)

arg = argparse.ArgumentParser()
arg.add_argument(
    "-u", "--url", type=str, help="URL", required=True
)
arg.add_argument(
    "-m", "--method", type=str, help="Request method", required=False, default="get"
)
arg.add_argument(
    "-a", "--params", type=str, help="Request parameters", required=False, default="{}"
)
arg.add_argument(
    "-e", "--email", type=str, help="email", required=False, default=""
)
arg.add_argument(
    "-p", "--password", type=str, help="password", required=False, default=""
)

namespace = arg.parse_args()

session = requests.Session()

if namespace.email and namespace.password:
    # login first

    login_url = "http://openjudge.cn/api/auth/login/"

    login_data = {
        "email": namespace.email,
        "password": namespace.password,
    }
    response = session.post(login_url, data=login_data)

    if response.status_code == 200:
        try:
            result = response.json()
            if result.get("result") != "SUCCESS":
                sys.stderr.write(result.get("message"))
                exit(1)
        except ValueError:
            sys.stderr.write("Could not parse JSON response from login.")
            exit(1)
    else:
        sys.stderr.write("OJ returns bad status code: " + str(response.status_code) + " when trying to login.")
        exit(1)

if namespace.method == "get":
    response = session.get(namespace.url)
else:
    params = eval(namespace.params)
    if not isinstance(params, dict):
        sys.stderr.write("Request params must be a dictionary.")
        exit(1)
    response = session.post(namespace.url, data=params)

if response.status_code == 200:
    print(response.text)
else:
    sys.stderr.write("Request failed with status code", response.status_code)
    exit(1)
