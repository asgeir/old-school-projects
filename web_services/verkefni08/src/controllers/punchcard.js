import async from 'async';
import express from 'express';
import Company from '../models/company';
import Punchcard from '../models/punchcard';
import User from '../models/user';
import {isAuthenticatedUser} from '../authenticate';

function handleIdParam(req, res, next, id) {
  Punchcard.findById(id, (err, punchcard) => {
    if (err) {
      return res.status(404).send('Punchcard not found: ' + err);
    }

    if (!punchcard) {
      return res.status(500).send('No Punchcard object returned');
    }

    req.punchcard = punchcard;
    return next();
  });
}

function handleCompanyIdParam(req, res, next, id) {
  Company.findById(id, (err, company) => {
    if (err) {
      return res.status(404).send('Company not found: ' + err);
    }

    if (!company) {
      return res.status(500).send('No Company object returned');
    }

    req.company = company;
    return next();
  });
}

function handleGet(req, res, next) {
  Punchcard.find({}, (err, punchcards) => {
    if (err || !punchcards) {
      return res.status(500).send('Error when retreiving punchcard list: ' + err);
    }

    return res.send([for (p of punchcards) p.toObject()]);
  });
}

function handleGetById(req, res, next) {
  res.send(req.punchcard);
}

function handleCreate(req, res, next) {
  async.waterfall([
    (cb) => {
      let companyId = req.company._id;
      let userId = req.authenticated_user._id;

      let earliestValidDate = new Date(Date.now);
      earliestValidDate.setDate(earliestValidDate.getDate() - req.company.punchcard_lifetime);

      Punchcard.find({ company_id: companyId, user_id: userId, created: { '$gte': earliestValidDate } }, (err, p) => {
        if (err) {
          res.status(500).send('Error when searching for old punchcards: ' + err);
          return cb(true);
        }

        if (p && (p.length > 0)) {
          res.status(409).send('A valid punchcard already exists.');
          return cb(true);
        }

        return cb(null);
      });
    },

    (cb) => {
      let p = new Punchcard({
        company_id: req.company._id,
        user_id: req.authenticated_user._id
      });
      p.save((err) => {
        if (err) {
          res.status(500).send('Error when creating punchcard: ' + err);
          return cb(true);
        }

        res.status(201).send({ id: p._id });
        return cb(null);
      });
    }
  ]);
}

const punchcardRouter = express.Router();

punchcardRouter.param('id', handleIdParam);
punchcardRouter.param('company_id', handleCompanyIdParam);
punchcardRouter.get('/', handleGet);
punchcardRouter.post('/:company_id', isAuthenticatedUser, handleCreate);
punchcardRouter.get('/:id', handleGetById);

export default punchcardRouter;
