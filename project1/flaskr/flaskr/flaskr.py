import os, sys

from flask import Flask


# From PEP 333: http://www.python.org/dev/peps/pep-0333/
############### BEGIN WSGI WRAPPER ##############
def run_with_cgi(application):

    environ = dict(os.environ.items())
    environ['wsgi.input']        = sys.stdin
    environ['wsgi.errors']       = sys.stderr
    environ['wsgi.version']      = (1, 0)
    environ['wsgi.multithread']  = False
    environ['wsgi.multiprocess'] = True
    environ['wsgi.run_once']     = True
    environ['SERVER_NAME'] = "127.0.0.1"
    environ['SERVER_PORT'] = "9999"
    environ['SERVER_PROTOCOL'] = "HTTP/1.1"
    environ['REQUEST_METHOD'] = 'GET'
    environ['PATH_INFO'] = "/"


    if environ.get('HTTPS', 'off') in ('on', '1'):
        environ['wsgi.url_scheme'] = 'https'
    else:
        environ['wsgi.url_scheme'] = 'http'

    headers_set = []
    headers_sent = []

    def write(data):
        if not headers_set:
             raise AssertionError("write() before start_response()")

        elif not headers_sent:
             # Before the first output, send the stored headers
             status, response_headers = headers_sent[:] = headers_set
             http_version = environ.get('SERVER_PROTOCOL', 'HTTP/1.1')
             http_connection = environ.get('HTTP_CONNECTION','close')
             sys.stdout.write('%s %s\r\n' % (http_version, status))
             sys.stdout.write('Connection: %s\r\n' % (http_connection))
             for header in response_headers:
                 sys.stdout.write('%s: %s\r\n' % header)
             sys.stdout.write('\r\n')

        sys.stdout.write(data.decode('utf8'))
        sys.stdout.flush()

    def start_response(status, response_headers, exc_info=None):
        if exc_info:
            try:
                if headers_sent:
                    # Re-raise original exception if headers sent
                    raise exc_info[0](exc_info[1])#, exc_info[2]
            finally:
                exc_info = None     # avoid dangling circular ref
        elif headers_set:
            raise AssertionError("Headers already set!")

        headers_set[:] = [status, response_headers]
        return write

    result = application(environ, start_response)
    try:
        for data in result:
            if data:    # don't send headers until body appears
                write(data)
        if not headers_sent:
            write('')   # send headers now if body was empty
    finally:
        if hasattr(result, 'close'):
            result.close()
############### END WSGI WRAPPER ##############



# create and configure the app
app = Flask(__name__, instance_relative_config=True)
app.config.from_mapping(
    SECRET_KEY='dev',
    DATABASE=os.path.join(app.instance_path, 'flaskr.sqlite'),
)


# ensure the instance folder exists
try:
    os.makedirs(app.instance_path)
except OSError:
    pass

# a simple page that says hello
@app.route('/hello')
def hello():
    return 'Hello, World!'

from .db import init_app
init_app(app)

from .auth import bp
app.register_blueprint(bp)


from .blog import 
app.register_blueprint(blog.bp)
app.add_url_rule('/', endpoint='index')



if __name__ == '__main__':
    run_with_cgi(app)