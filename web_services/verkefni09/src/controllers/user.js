import express from 'express';
import User from '../models/user';
import {HighLevelProducer} from 'kafka-node';
import {getClient as kafkaGetClient} from '../messages';
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

let kafkaProducer = new HighLevelProducer(kafkaGetClient('user-router'));
function dispatchUserMessage(body) {
  const messageData = [{ topic: 'users', messages: JSON.stringify(body) }];
  kafkaProducer.send(messageData, (err) => {
    if (err) {
      console.log('Error:', err);
    }
  });
}

function handleCreate(req, res, next) {
  if (!req.body.username) {
    return res.status(403).send('User field "username" is requried');
  } else if (!req.body.email) {
    return res.status(403).send('User field "email" is requried');
  } else if (!req.body.password) {
    return res.status(403).send('User field "password" is requried');
  } else if (!req.body.age) {
    return res.status(403).send('User field "age" is requried');
  } else if (!req.body.gender) {
    return res.status(403).send('User field "gender" is requried');
  }

  let u = new User({
    username: req.body.username,
    email: req.body.email,
    password: req.body.password,
    age: req.body.age,
    gender: req.body.gender
  });
  u.save({ validateBeforeSave: true }, (err) => {
    if (err) {
      if (err.name == 'ValidationError') {
        return res.status(409).send('Validation error: ' + err.message);
      } else if (11000 === err.code || 11001 === err.code) {
        return res.status(409).send('Username or email already registered');
      }

      return res.status(500).send('Error when creating user: ' + err);
    }

    dispatchUserMessage(req.body);

    return res.status(201).send(u);
  });
}

const userRouter = express.Router();

userRouter.param('id', handleIdParam);
userRouter.get('/', handleGet);
userRouter.post('/', isAuthenticatedAdmin, handleCreate);
userRouter.get('/:id', handleGetById);

export default userRouter;
