import express from 'express';
import Company from '../models/company';
import {isAuthenticatedAdmin} from '../authenticate';

function handleIdParam(req, res, next, id) {
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
  Company.find({}, (err, companies) => {
    if (err || !companies) {
      return res.status(500).send('Error when retreiving company list: ' + err);
    }

    return res.send([for (c of companies) c.toObject()]);
  });
}

function handleGetById(req, res, next) {
  res.send(req.company);
}

function handleCreate(req, res, next) {
  if (!req.body.name) {
    return res.status(412).send('Company field "name" is requried');
  } else if (!req.body.description) {
    return res.status(412).send('Company field "description" is requried');
  } else if ((!req.body.punchcard_lifetime) || (parseInt(req.body.punchcard_lifetime) <= 0)) {
    return res.status(412).send('Company field "punchcard_lifetime" is requried and must be a positive integer');
  }

  let c = new Company({
    name: req.body.name,
    description: req.body.description,
    punchcard_lifetime: req.body.punchcard_lifetime
  });
  c.save((err) => {
    if (err) {
      return res.status(500).send('Error when creating company: ' + err);
    }

    return res.status(201).send({ company_id: c._id });
  });
}

const companyRouter = express.Router();

companyRouter.param('id', handleIdParam);
companyRouter.get('/', handleGet);
companyRouter.post('/', isAuthenticatedAdmin, handleCreate);
companyRouter.get('/:id', handleGetById);

export default companyRouter;
