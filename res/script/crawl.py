import argparse

try:
    import requests
except ImportError:
    raise ImportError("Please install requests module: pip install requests")

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
                raise Exception("Login Failed! ", result.get("message"))
        except ValueError:
            raise Exception("Could not parse JSON response from login.")
    else:
        raise Exception("OJ returns bad status code", response.status_code, "when trying to login.")

if namespace.method == "get":
    response = session.get(namespace.url)
else:
    params = eval(namespace.params)
    if not isinstance(params, dict):
        raise Exception("Request params must be a dictionary.")
    response = session.post(namespace.url, data=params)

if response.status_code == 200:
    print(response.text)
else:
    raise Exception("Request failed with status code", response.status_code)
