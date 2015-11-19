import 'babel/polyfill';
import express from 'express';
import http from 'http';
import debug from 'debug';
import mongoose from 'mongoose';
import morgan from 'morgan';
import bodyParser from 'body-parser';
import {initAuthentication} from './authenticate';
import {initMessages} from './messages';
import {initDaemons} from './daemons';

const app = express();

// connect to database
let dbOptions = {
  server: {
    socketOptions: { keepAlive: 1 }
  }
};

switch (app.get('env')) {
case 'development':
  mongoose.connect('mongodb://localhost/asgeirb09verkefni9', dbOptions);
  break;

case 'production':
  mongoose.connect(process.env.DATABASE_URL, dbOptions);
  break;

default:
  throw new Error('Unknown execution environment: ' + app.get('env'));
}

// initialize message queue
switch (app.get('env')) {
case 'development':
  initMessages('localhost:2181');
  break;

case 'production':
  initMessages(process.env.MESSAGES_CONNECTION_STRING);
  break;

default:
  throw new Error('Unknown execution environment: ' + app.get('env'));
}

// initialize authentication
switch (app.get('env')) {
case 'development':
  initAuthentication('admin-token');
  break;

case 'production':
  initAuthentication(process.env.ADMIN_TOKEN);
  break;

default:
  throw new Error('Unknown execution environment: ' + app.get('env'));
}

// initialize daemons
initDaemons({});

// Set additional headers
app.disable('x-powered-by');
app.use((req, res, next) => { res.set('X-Clacks-Overhead', 'GNU Terry Pratchett'); return next(); });

// setup middleware
app.use(morgan('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

// setup routes
let apiRouter = express.Router();
app.use('/api', apiRouter);
apiRouter.use('/company', require('./controllers/company'));
apiRouter.use('/punchcard', require('./controllers/punchcard'));
apiRouter.use('/user', require('./controllers/user'));
apiRouter.use('/token', require('./controllers/token'));

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function (err, req, res) {
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function (err, req, res) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});

/**
 * Create HTTP server.
 */

const port = (process.env.PORT || '3000');
app.set('port', port);

const server = http.createServer(app);

/**
 * Listen on provided port, on all network interfaces.
 */

server.listen(port);
server.on('error', onError);
server.on('listening', onListening);

/**
 * Event listener for HTTP server "error" event.
 */

function onError(error) {
  if (error.syscall !== 'listen') {
    throw error;
  }

  var bind = typeof port === 'string'
    ? 'Pipe ' + port
    : 'Port ' + port;

  // handle specific listen errors with friendly messages
  switch (error.code) {
  case 'EACCES':
    console.error(bind + ' requires elevated privileges');
    throw error;
  case 'EADDRINUSE':
    console.error(bind + ' is already in use');
    throw error;
  default:
    throw error;
  }
}

/**
 * Event listener for HTTP server "listening" event.
 */

function onListening() {
  var addr = server.address();
  var bind = typeof addr === 'string'
    ? 'pipe ' + addr
    : 'port ' + addr.port;
  debug('Listening on ' + bind);
}
