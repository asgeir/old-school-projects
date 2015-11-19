import mongoose from 'mongoose';

const companySchema = new mongoose.Schema({
  name: { type: String, trim: true },
  description: { type: String, trim: true },
  punchcard_lifetime: { type: Number, min: 0 }
});

if (!companySchema.options.toJSON) { companySchema.options.toJSON = {}; }
if (!companySchema.options.toObject) { companySchema.options.toObject = {}; }
companySchema.options.toJSON.transform = companySchema.options.toObject.transform = (doc, ret) => {
  delete ret.__v;
};

const Company = mongoose.model('Company', companySchema);

export default Company;
