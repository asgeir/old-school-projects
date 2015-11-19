import express from 'express';
import Company from '../models/company';
import {isAuthenticatedAdmin} from '../authenticate';
import {client as searchClient} from '../search';

function handleIdParam(req, res, next, id) {
  if (!/^[a-f0-9]{24}$/.test(id)) return next();

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
  let from = parseInt(req.query.page) || 0;
  let size = parseInt(req.query.max) || 20;

  searchClient.search({
    index: 'company',
    from: from * size,
    size: size,
    sort: 'title.raw:asc'
  }).then((body) => {
    res.send([for (hit of body.hits.hits) {
      id: hit._source.id,
      title: hit._source.title,
      description: hit._source.description,
      url: hit._source.url
    }]);
  }).catch((err) => {
    res.status(500).send('Error fetching company list: ' + err);
  });
}

function handleGetById(req, res, next) {
  let data = req.company.toObject();
  delete data.punchcard_lifetime;
  delete data.created;

  res.send(data);
}

function handleCreate(req, res, next) {
  if (!req.body.title) {
    return res.status(412).send('Company field "title" is requried');
  } else if (!req.body.description) {
    return res.status(412).send('Company field "description" is requried');
  } else if ((!req.body.punchcard_lifetime) || (parseInt(req.body.punchcard_lifetime) <= 0)) {
    req.body.punchcard_lifetime = 1;
  }

  Company.findOne({ title: req.body.title }, (err, companyExists) => {
    if (err) {
      return res.status(500).send('Error when checking if company exists: ' + err);
    } else if (companyExists) {
      return res.status(409).send('A company with that title already exists');
    }

    let c = new Company({
      title: req.body.title,
      url: req.body.url,
      description: req.body.description,
      punchcard_lifetime: req.body.punchcard_lifetime
    });
    c.save((dbErr) => {
      if (dbErr) {
        return res.status(500).send('Error when creating company: ' + dbErr);
      }

      searchClient.create({
        index: 'company',
        type: 'company',
        id: c._id.toString(),
        body: c.toObject()
      }).catch((searchErr) => {
        console.log('Error inserting into index: ' + searchErr);
      });

      return res.status(201).send({ id: c._id });
    });
  });
}

function handleUpdate(req, res, next) {
  if (!req.body.title) {
    return res.status(412).send('Company field "title" is requried');
  } else if (!req.body.description) {
    return res.status(412).send('Company field "description" is requried');
  } else if ((!req.body.punchcard_lifetime) || (parseInt(req.body.punchcard_lifetime) <= 0)) {
    req.body.punchcard_lifetime = 1;
  }

  Company.findOne({ title: req.body.title }, (err, companyExists) => {
    if (err) {
      return res.status(500).send('Error when checking if company exists: ' + err);
    } else if (companyExists) {
      return res.status(409).send('A company with that title already exists');
    }

    if (req.body.title) req.company.title = req.body.title;
    if (req.body.url) req.company.url = req.body.url;
    if (req.body.description) req.company.description = req.body.description;
    req.company.save(() => {
      searchClient.update({
        index: 'company',
        type: 'company',
        id: req.company._id.toString(),
        body: {
          doc: req.company.toObject()
        }
      }).then(() => {
        res.status(202).send({ id: req.company._id });
      }).catch((searchErr) => {
        console.log('Error inserting into index: ' + searchErr);
        searchErr.status = 500;
        next(searchErr);
      });
    });
  });
}

function handleDelete(req, res, next) {
  let id = req.company._id.toString();

  req.company.remove(() => {
    searchClient.delete({
      index: 'company',
      type: 'company',
      id
    });

    res.send({ status: 'deleted' });
  });
}

function handleSearch(req, res, next) {
  searchClient.search({
    index: 'company',
    q: req.body.search
  }).then((body) => {
    res.send([for (hit of body.hits.hits) {
      id: hit._source.id,
      title: hit._source.title,
      description: hit._source.description,
      url: hit._source.url
    }]);
  }).catch((err) => {
    res.status(500).send('Error fetching company list: ' + err);
  });
}

function requireJSON(req, res, next) {
  if (req.is('application/json')) {
    return next();
  }

  return res.status(415).send('Data must be JSON formatted');
}

const companyRouter = express.Router();

companyRouter.param('id', handleIdParam);
companyRouter.get('/', handleGet);
companyRouter.post('/', isAuthenticatedAdmin, requireJSON, handleCreate);
companyRouter.post('/search', handleSearch);
companyRouter.get('/:id', handleGetById);
companyRouter.post('/:id', isAuthenticatedAdmin, requireJSON, handleUpdate);
companyRouter.delete('/:id', isAuthenticatedAdmin, handleDelete);

export default companyRouter;
