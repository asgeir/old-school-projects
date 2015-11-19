import express from 'express';
import User from '../models/user';
import {isAuthenticatedAdmin} from '../authenticate';

function handleIdParam(req, res, next, id) {
  User.findById(id, (err, user) => {
    if (err) {
      return res.status(404).send('User not found: ' + err);
    }

    if (!user) {
      return res.status(500).send('No User object returned');
    }

    req.target_user = user;
    return next();
  });
}

function handleGet(req, res, next) {
  User.find({}, (err, users) => {
    if (err || !users) {
      return res.status(500).send('Error when retreiving user list: ' + err);
    }

    return res.send([for (u of users) u.toObject()]);
  });
}

function handleGetById(req, res, next) {
  res.send(req.target_user);
}

function handleCreate(req, res, next) {
  if (!req.body.name) {
    return res.status(403).send('User field "name" is requried');
  } else if (!req.body.age) {
    return res.status(403).send('User field "age" is requried');
  } else if (!req.body.gender) {
    return res.status(403).send('User field "gender" is requried');
  }

  let u = new User({
    name: req.body.name,
    age: req.body.age,
    gender: req.body.gender
  });
  u.save((err) => {
    if (err) {
      return res.status(500).send('Error when creating user: ' + err);
    }

    return res.status(202).send(u);
  });
}

const userRouter = express.Router();

userRouter.param('id', handleIdParam);
userRouter.get('/', handleGet);
userRouter.post('/', isAuthenticatedAdmin, handleCreate);
userRouter.get('/:id', handleGetById);

export default userRouter;
