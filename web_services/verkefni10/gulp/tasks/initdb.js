import gulp from 'gulp';
import async from 'async';
import mongoose from 'mongoose';
import User from '../../src/models/user';

function initdbUser(callback) {
  new User({
    username: 'testuser',
    email: 'test@user.com',
    password: 'test',
    token: 'test-token',
    age: 200,
    gender: 'o'
  }).save(callback);
}

export default (plugin, src, config) => {
  gulp.task('initdb', function (taskDone) {
    mongoose.connect(config.initdb.connectionString, {
      server: {
        socketOptions: { keepAlive: 1 }
      }
    });

    async.series([
      initdbUser,
      () => { taskDone(); }
    ]);
  });
};
