from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib #Only for parse.unquote and parse.unquote_plus.
import json
import base64
from typing import Union, Optional
import re
# If you need to add anything above here you should check with course staff first.
import csv
import os

USERNAME = "admin"
PASSWORD = "password"

CONTACTS = [{0: {
    'name': "user",
    'email': "user@domain.com",
    'date': "0000-00-00",
    'dropdown': "N/A",
    'checkbox': "N/A"
}}]
NEXT_ID = 1
LABELS = [*CONTACTS[0][0]]
REQ_FIELDS = {'name': 0, 'email': 0}
ENDPOINTS = []
IMG_TYPES = {}
SALE = {'active': False, 'message': None}

def get_endpoints(filename: str):
    global ENDPOINTS
    with open(filename, newline='') as f:
        reader = csv.DictReader(f)
        ENDPOINTS = [row for row in reader]
get_endpoints('endpoints.txt')

def get_images(dirname: str):
    global IMG_TYPES
    imgs = os.listdir(dirname)
    for img in imgs:
        if '.' in img: img = img.split('.'); IMG_TYPES = {**IMG_TYPES, **{img[0]: img[1]}}
get_images("./static/images")

def parse_params(params):
    if (params == None): return error_html, 400

    params = params.split('&')
    params = [param.split('=') for param in params if '=' in param]
    for param in params: param[1] = urllib.parse.unquote(param[1])

    # Check for extra keys
    for param in params:
        if param[0] not in LABELS: return error_html, 400 
    
    # Check for required keys
    for fld in [*REQ_FIELDS]:
        for i in range(len(params)):
            if fld == params[i][0]: REQ_FIELDS[fld] += 1

    # Reset for new parameters
    FLAG = False
    for k, v in REQ_FIELDS.items():
        if v != 1: FLAG = True
        REQ_FIELDS[k] = 0
    if (FLAG): return error_html, 400 

    contact = {}
    if len(params) == len(LABELS):
        # Check for correct order
        for i in range(len(params)):
            if params[i][0] != LABELS[i]: return error_html, 400 
        for param in params: contact[param[0]] = param[1]

    # Less parameters, more form fields
    elif len(params) < len(LABELS):
        (p_idx, l_idx, missing_k) = (0, 0, {})
        FLAG = False
        while (p_idx < len(params) and l_idx < len(LABELS)):
            (params_key, params_val, labels_key) = (params[p_idx][0], params[p_idx][1], LABELS[l_idx])
            # Fill missing keys (un-filled form fields)
            if params_key != labels_key: missing_k[labels_key] = ""; l_idx+=1
            else: missing_k[params_key] = params_val; p_idx+=1; l_idx+=1
            
            if p_idx >= len(params) and l_idx < len(LABELS): FLAG = True; p_idx-=1
        # Incorrect order of parameters in url; return because of tampering
        # e. ?email=email&checkbox=yes&name=name
        if not FLAG and p_idx < len(params) and l_idx >= len(LABELS): return error_html, 400 

        # Check for correct order, after filling missing keys
        k_cpy = [*missing_k]
        for i in range(len(k_cpy)):
            if k_cpy[i] != LABELS[i]: return error_html, 400 
        contact = {**contact, **missing_k}

    else: return error_html, 400 

    # empty fields for REQs --- APOLOGIES FOR BAD CODE WITHIN THIS FUNC
    for k in [*REQ_FIELDS]:
        if (len(contact[k]) == 0): return error_html, 400
    
    global NEXT_ID
    contact = {NEXT_ID: contact}
    NEXT_ID+=1

    CONTACTS.append(contact); print(f"{CONTACTS} --- [Length: {len(CONTACTS)}]")
    return confirmation_html, 201

def error_html():
    nav_content = ""
    html_path = "../"
    nav_pages = {
        "main": "GreenWrap Solutions",
        "testimonies": "Testimonies",
        "contact": "Contact Us",
        "admin/contactlog": "Contact Log"
    }
    nav_links = ""
    for k, v in nav_pages.items(): nav_links += f"<a href=\"{html_path+k}\">{v}</a>"
    nav_content += nav_links
    nav_content += "<button type=\"button\" class=\"mode\">Theme</button>"

    error_msg = "<img id=\"error_gif\" alt=\"Gif representing error message\" src=\"../images/error.gif\">"

    body_content = ""
    body_content += f"<nav>{nav_content}</nav>"
    body_content += f"{error_msg}"

    script = "<script defer src=\"/js/main.js\"></script>"

    head_content = f"<meta charset=\"UTF-8\"><title>Failed!</title><link rel=\"stylesheet\" href=\"/main.css\">{script}"
    html = f"<!DOCTYPE html><html lang=\"en\"><head>{head_content}</head><body>{body_content}</body></html>"
    return html

def confirmation_html():
    nav_content = ""
    html_path = "../"
    nav_pages = {
        "main": "GreenWrap Solutions",
        "testimonies": "Testimonies",
        "contact": "Contact Us",
        "admin/contactlog": "Contact Log"
    }
    nav_links = ""
    for k, v in nav_pages.items(): nav_links += f"<a href=\"{html_path+k}\">{v}</a>"
    nav_content += nav_links
    nav_content += "<button type=\"button\" class=\"mode\">Theme</button>"

    confirm_msg = "<img id=\"confirm_gif\" alt=\"Gif representing confirmation message\" src=\"../images/confirm.gif\">"

    body_content = ""
    body_content += f"<nav>{nav_content}</nav>"
    body_content += f"{confirm_msg}"

    script = "<script defer src=\"/js/main.js\"></script>"

    head_content = f"<meta charset=\"UTF-8\"><title>Success!</title><link rel=\"stylesheet\" href=\"/main.css\">{script}"
    html = f"<!DOCTYPE html><html lang=\"en\"><head>{head_content}</head><body>{body_content}</body></html>"
    return html

def contacts_to_html():
    nav_content = ""
    html_path = "../"
    nav_pages = {
        "main": "GreenWrap Solutions",
        "testimonies": "Testimonies",
        "contact": "Contact Us",
        "admin/contactlog": "Contact Log"
    }
    nav_links = ""
    for k, v in nav_pages.items(): nav_links += f"<a href=\"{html_path+k}\">{v}</a>"
    nav_content += nav_links
    nav_content += "<button type=\"button\" class=\"mode\">Theme</button>"

    set_text = "<label for=\"sale_text\">Text: </label><input type=\"text\" id=\"sale_text\" name=\"sale_text\" required>"
    set_btn = "<button type=\"button\" id=\"setSaleButton\">Set</button>"
    del_btn = "<button type=\"button\" id=\"delSaleButton\">Delete</button>"
    sale_content = f"<p id=\"sale_p\">{set_text}</p><p><span>{set_btn}</span><span>{del_btn}</span></p>"

    table_header = "<tr>"
    for label in LABELS:
        label = label.replace(label, "Status") if (label == "checkbox") else label.replace(label, "Service") if (label == "dropdown") else label.capitalize()
        table_header += f"<th>{label}</th>"
    table_header += "<th>Time Until</th>"
    table_header += "<th>Delete Row</th>"
    table_header += "</tr>"

    table_data = ""
    for contact_dict in CONTACTS:
        i = next(iter(contact_dict))
        table_data += f"<tr id=\"{i}\">"
        for k, v in contact_dict[i].items():
            if k == "date": table_data += f"<td class=\"date\">{v}</td>"
            else:
                if k == "name":
                    if ('<' in v): v = v.replace('<', "&lt;")
                if k == "email": v = f"<a href=\"mailto:{v}\">{v}</a>"
                if k == "checkbox": v = "Not Subscribed" if (v == "") else "Subscribed"
                table_data += f"<td>{v}</td>"
        table_data += "<td class=\"timeUntil\"></td>"
        table_data += "<td><button type=\"button\" class=\"deleteRowButton\">Click Me!</button></td>"
        table_data += "</tr>"
    table_content = "<caption>Contact Information</caption>" + table_header + table_data

    body_content = f"<nav>{nav_content}</nav>"
    body_content += f"<div id=\"set_sale\">{sale_content}</div>"
    body_content += f"<div id=\"contact_log\"><table>{table_content}</table></div>"

    links = "<link rel=\"stylesheet\" href=\"/main.css\">"
    scripts = "<script defer src=\"/js/table.js\"></script><script defer src=\"/js/main.js\"></script>"

    head_content = f"<meta charset=\"UTF-8\"><title>Contact Log</title>{links}{scripts}"
    html = f"<!DOCTYPE html><html lang=\"en\"><head>{head_content}</head><body>{body_content}</body></html>"
    return html

def authenticate(endpoint: str, headers: dict[str, str]):
    if ("Authorization" not in headers): return [
        False,
        ("Unauthorized", 401, { 'Content-Type': "text/html", 'WWW-Authenticate': "Basic realm=\"Access to admin site\""})
    ]
    encoded = headers['Authorization'].split(' ', 1)[1]
    decoded = base64.b64decode(encoded).decode("utf-8")
    decoded = decoded.split(':', 1)
    if (decoded[0]==USERNAME and decoded[1]==PASSWORD):
        return [True, None]
    return[False, ("Forbidden", 403, {'Content-Type': "text/html"})]

# The method signature is a bit "hairy", but don't stress it -- just check the documentation below.
def server(method: str, url: str, body: Optional[str], headers: dict[str, str]) -> tuple[Union[str, bytes], int, dict[str, str]]:    
    """
    method will be the HTTP method used, for our server that's GET, POST, DELETE
    url is the partial url, just like seen in previous assignments
    body will either be the python special None (if the body wouldn't be sent)
         or the body will be a string-parsed version of what data was sent.
    headers will be a python dictionary containing all sent headers.

    This function returns 3 things:
    The response body (a string containing text, or binary data)
    The response code (200 = ok, 404=not found, etc.)
    A _dictionary_ of headers. This should always contain Content-Type as seen in the example below.
    """
    # Parse URL -- this is probably the best way to do it. Delete if you want.
    url, *parameters = url.split("?", 1)

    # GET
    if (method == "GET"):
        # html .css js/ pages
        for endpoint in ENDPOINTS:
            if endpoint['path'] == url:
                # dynamic page
                if (url == "/admin/contactlog"):
                    ret = authenticate(url, headers)
                    if (not ret[0]): return ret[1]
                    return contacts_to_html(), (int)(endpoint['status']), {'Content-Type': endpoint['type']}
                # static page
                return open(f"static{endpoint['file']}").read(), (int)(endpoint['status']), {'Content-Type': endpoint['type']}

        # /images/main
        if (url == "/images/main"): return open("static/images/logo.png", "rb").read(), 200, {'Content-Type': "image/png"}
        for img, typ in IMG_TYPES.items():
            if (url == "/images/" + f"{img}.{typ}"): return open("static" + url, "rb").read(), 200, {'Content-Type': f"image/{typ}"}
        
        # /api/sale
        if (url == "/api/sale"): return json.dumps(SALE), 200, {'Content-Type': "application/json"}

    # POST
    if (method == "POST"):
        # /contact
        if (url == "/contact"):
            foo, status_code = parse_params(body)
            return foo(), status_code, {'Content-Type': "text/html"}
        
        # /api/sale
        if (url == "/api/sale"):
            ret = authenticate(url, headers)
            if (not ret[0]): return ret[1]

            cases = True
            cases &= ('Content-Type' in headers)
            cases &= (headers['Content-Type'] == "application/json")

            try:
                json_dict = json.loads(body)
            except:
                return "400 Bad Request", 400, {'Content-Type': "text/plain"}
            
            cases &= ('message' in json_dict)
            if (not cases): return "400 Bad Request", 400, {'Content-Type': "text/plain"}
            SALE['active'] = True; SALE['message'] = json_dict['message']
            return "200 OK", 200, {'Content-Type': "text/plain"}
    
    # DELETE
    if (method == "DELETE"):
        # /api/contact
        if (url == "/api/contact"):
            ret = authenticate(url, headers)
            if (not ret[0]): return ret[1]

            cases = True
            cases &= ('Content-Type' in headers)
            cases &= (headers['Content-Type'] == "application/json")

            try:
                json_dict = json.loads(body)
            except:
                return "400 Bad Request", 400, {'Content-Type': "text/plain"}

            cases &= ('id' in json_dict)

            # invalid inputs
            if (not cases): return "400 Bad Request", 400, {'Content-Type': "text/plain"}

            FLAG = False
            for contact in CONTACTS:
                if (int)(json_dict['id']) in contact: FLAG = True
            
            # input is valid, but no contact with given id
            if (not FLAG): return "404 Not Found", 404, {'Content-Type': "text/plain"}

            # remove contact
            for contact_dict in CONTACTS:
                id = next(iter(contact_dict))
                if (id == (int)(json_dict['id'])): CONTACTS.remove(contact_dict); break

            print(f"{CONTACTS} --- [Length: {len(CONTACTS)}]\n")
            return "200 OK", 200, {'Content-Type': "text/plain"}
        
        # /api/sale
        if (url == "/api/sale"):
            ret = authenticate(url, headers)
            if (not ret[0]): return ret[1]

            SALE['active'] = False; SALE['message'] = None
            return "200 OK", 200, {'Content-Type': "text/plain"}

    # And another freebie -- the 404 page will probably look like this.
    # Notice how we have to be explicit that "text/html" should be the value for
    # header: "Content-Type" now?]
    # I am sorry that you're going to have to do a bunch of boring refactoring.
    return open("static/html/404.html").read(), 404, {"Content-Type": "text/html; charset=utf-8"}


# You shouldn't need to change content below this. It would be best if you just left it alone.


class RequestHandler(BaseHTTPRequestHandler):
    def c_read_body(self):
        # Read the content-length header sent by the BROWSER
        content_length = int(self.headers.get("Content-Length", 0))
        # read the data being uploaded by the BROWSER
        body = self.rfile.read(content_length)
        # we're making some assumptions here -- but decode to a string.
        body = str(body, encoding="utf-8")
        return body

    def c_send_response(self, message, response_code, headers):
        # Convert the return value into a byte string for network transmission
        if type(message) == str:
            message = bytes(message, "utf8")
        
        # Send the first line of response.
        self.protocol_version = "HTTP/1.1"
        self.send_response(response_code)
        
        # Send headers (plus a few we'll handle for you)
        for key, value in headers.items():
            self.send_header(key, value)
        self.send_header("Content-Length", len(message))
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()

        # Send the file.
        self.wfile.write(message)
        

    def do_POST(self):
        # Step 1: read the last bit of the request
        try:
            body = self.c_read_body()
        except Exception as error:
            # Can't read it -- that's the client's fault 400
            self.c_send_response("Couldn't read body as text", 400, {'Content-Type':"text/plain"})
            raise
                
        try:
            # Step 2: handle it.
            message, response_code, headers = server("POST", self.path, body, self.headers)
            # Step 3: send the response
            self.c_send_response(message, response_code, headers)
        except Exception as error:
            # If your code crashes -- that's our fault 500
            self.c_send_response("The server function crashed.", 500, {'Content-Type':"text/plain"})
            raise
        

    def do_GET(self):
        try:
            # Step 1: handle it.
            message, response_code, headers = server("GET", self.path, None, self.headers)
            # Step 3: send the response
            self.c_send_response(message, response_code, headers)
        except Exception as error:
            # If your code crashes -- that's our fault 500
            self.c_send_response("The server function crashed.", 500, {'Content-Type':"text/plain"})
            raise


    def do_DELETE(self):
        # Step 1: read the last bit of the request
        try:
            body = self.c_read_body()
        except Exception as error:
            # Can't read it -- that's the client's fault 400
            self.c_send_response("Couldn't read body as text", 400, {'Content-Type':"text/plain"})
            raise
        
        try:
            # Step 2: handle it.
            message, response_code, headers = server("DELETE", self.path, body, self.headers)
            # Step 3: send the response
            self.c_send_response(message, response_code, headers)
        except Exception as error:
            # If your code crashes -- that's our fault 500
            self.c_send_response("The server function crashed.", 500, {'Content-Type':"text/plain"})
            raise



def run():
    PORT = 4131
    print(f"Starting server http://localhost:{PORT}/")
    server = ("", PORT)
    httpd = HTTPServer(server, RequestHandler)
    httpd.serve_forever()


run()