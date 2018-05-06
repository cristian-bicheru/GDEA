import webapp2


class MainPage(webapp2.RequestHandler):
    def get(self):
        self.response.headers['Content-Type'] = 'text/plain'
        self.redirect('/static/index.html')


app = webapp2.WSGIApplication([
    ('/', MainPage),
], debug=True)
