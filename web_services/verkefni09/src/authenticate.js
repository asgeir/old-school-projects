import User from './models/user';

let ADMIN_TOKEN = null;

export function initAuthentication(adminToken) {
  ADMIN_TOKEN = adminToken;
}

export function isAuthenticatedAdmin(req, res, next) {
  if (!ADMIN_TOKEN || req.header('ADMIN_TOKEN') !== ADMIN_TOKEN) {
    return res.status(401).send('Invalid admin token');
  }

  return next();
}

export function isAuthenticatedUser(req, res, next) {
  var token = req.header('TOKEN');
  if (!token) {
    return res.status(401).send('No token value found');
  }

  User.findByToken(token, (err, user) => {
    if (err) {
      return res.status(500).send('Error looking up user: ' + err);
    }

    if (!user || !user.isTokenValid()) {
      return res.status(401).send('Invalid user token');
    }

    req.authenticated_user = user;
    return next();
  });
}
