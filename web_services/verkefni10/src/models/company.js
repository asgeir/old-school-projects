import mongoose from 'mongoose';
import {client as searchClient} from '../search';

const companySchema = new mongoose.Schema({
  title: { type: String, trim: true, unique: true, minlength: 1 },
  url: { type: String, trim: true },
  description: { type: String, trim: true },
  punchcard_lifetime: { type: Number, min: 0 },
  created: { type: Date, default: Date.now }
});

if (!companySchema.options.toJSON) { companySchema.options.toJSON = {}; }
if (!companySchema.options.toObject) { companySchema.options.toObject = {}; }
companySchema.options.toJSON.transform = companySchema.options.toObject.transform = (doc, ret) => {
  ret.id = ret._id.toString();

  delete ret.__v;
  delete ret._id;
};

const Company = mongoose.model('Company', companySchema);

export function initSearch() {
  searchClient.indices.exists({
    index: 'company'
  }).then((indexExists) => {
    if (!indexExists) {
      searchClient.indices.create({
        index: 'company'
      }).then(() => {
        searchClient.indices.putMapping({
          index: 'company',
          type: 'company',
          body: {
            properties: {
              id: { type: 'string' },
              title: {
                type: 'string',
                fields: {
                  raw: {
                    type: 'string',
                    index: 'not_analyzed'
                  }
                }
              },
              url: { type: 'string' },
              description: { type: 'string' },
              punchcard_lifetime: { type: 'double' },
              created: { type: 'date' }
            }
          }
        }).catch((err) => {
          console.log('Unable to create company type mapping: ' + err);
        });
      }).catch((err) => {
        console.log('Unable to create company search index: ' + err);
      });
    }
  });
}

export default Company;
