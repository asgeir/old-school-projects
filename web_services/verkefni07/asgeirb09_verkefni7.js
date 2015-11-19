// Requires tools:
//     Babel transpiler
//
// Requires packages:
//     express
//     body-parser
//
// To run:
//     $ babel -e 0 asgeirb09_verkefni7.js | node

import express from 'express';
import bodyParser from 'body-parser';

let _companies = [];
class Company
{
  constructor(name, punchCount) {
    this.id = _companies.length;
    this.name = name;
    this.punchCount = punchCount;

    Company.companies().push(this);
  }

  static companies() {
    return _companies;
  }

  static setupRoute(parentRoute) {
    let router = express.Router();
    parentRoute.use('/companies', router);

    router.param('id', Company.handleIdParam);
    router.get('/', Company.handleGet);
    router.post('/', Company.handleCreate);
    router.get('/:id', Company.handleGetById);
  }

  static handleIdParam(req, res, next, id) {
    for (let c of Company.companies()) {
      if (c.id == parseInt(id)) {
        req.company = c;
        return next();
      }
    }

    res.status(404).send('Company not found');
  }

  static handleGet(req, res, next) {
    res.send(Company.companies());
  }

  static handleGetById(req, res, next) {
    res.send(req.company);
  }

  static handleCreate(req, res, next) {
    if (!req.body.name) {
      return res.status(403).send('Company field "name" is requried');
    } else if (!req.body.punchCount) {
      return res.status(403).send('Company field "punchCount" is requried');
    }

    let c = new Company(req.body.name, parseInt(req.body.punchCount));
    res.status(202).send(c);
  }
}

let _punches = [];
class Punch
{
  constructor(companyId) {
    this.id = _punches.length;
    this.companyId = companyId;
    this.date = new Date();

    Punch.punches().push(this);
  }

  static punches() {
    return _punches;
  }
}

let _users = [];
class User
{
  constructor(name, email) {
    this.id = _users.length;
    this.name = name;
    this.email = email;
    this.punches = [];

    User.users().push(this);
  }

  static users() {
    return _users;
  }

  static setupRoute(parentRoute) {
    let router = express.Router();
    parentRoute.use('/users', router);

    router.param('id', User.handleIdParam);
    router.get('/', User.handleGet);
    router.post('/', User.handleCreate);
    router.get('/:id', User.handleGetById);
    router.get('/:id/punches', User.handleGetPunches);
    router.post('/:id/punches', User.handleCreatePunch);
  }

  static handleIdParam(req, res, next, id) {
    for (let u of User.users()) {
      if (u.id == parseInt(id)) {
        req.target_user = u;
        return next();
      }
    }

    res.status(404).send('User not found');
  }

  static handleGet(req, res, next) {
    res.send(User.users());
  }

  static handleGetById(req, res, next) {
    res.send(req.target_user);
  }

  static handleCreate(req, res, next) {
    if (!req.body.name) {
      return res.status(403).send('User field "name" is requried');
    } else if (!req.body.email) {
      return res.status(403).send('User field "email" is requried');
    }

    let u = new User(req.body.name, req.body.email);
    res.status(202).send(u);
  }

  static handleGetPunches(req, res, next) {
    let punches = req.target_user.punches;
    if ((req.query.company != null) && (req.query.company != undefined)) {
      let companyId = parseInt(req.query.company);
      punches = [for (p of punches) if (p.companyId == companyId) p];
    }

    res.send(punches);
  }

  static handleCreatePunch(req, res, next) {
    if ((req.body.id == null) || (req.body.id == undefined)) {
      return res.status(403).send('Punch field "id" is requried');
    }

    let p = new Punch(parseInt(req.body.id));
    req.target_user.punches.push(p);
    res.status(202).send(req.target_user);
  }
}

let app = express();

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

let apiRouter = express.Router();
app.use('/api', apiRouter);
Company.setupRoute(apiRouter);
User.setupRoute(apiRouter);

let server = app.listen(3000, () => {
  const host = server.address().address;
  const port = server.address().port;

  console.log('Verkefni7 app listening at http://%s:%s', host, port);
});
