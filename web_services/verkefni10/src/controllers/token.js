import express from 'express';
import User from '../models/user';

function handleFetchToken(req, res, next) {
  User.findOne({ username: req.body.username }, (err, user) => {
    if (err) {
      return res.status(500).send('Error when looking for user: ' + err);
    }

    if (!user || !user.verifyPassword(req.body.password)) {
      return res.status(401).send('Username or password incorrect');
    }

    if (!user.isTokenValid()) {
      return res.status(403).send('User token has not been generated yet');
    }

    return res.send({ token: user.token });
  });
}

const tokenRouter = express.Router();

tokenRouter.post('/', handleFetchToken);

export default tokenRouter;
