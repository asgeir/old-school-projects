import mongoose from 'mongoose';

const punchcardSchema = new mongoose.Schema({
  company_id: { type: mongoose.Schema.Types.ObjectId },
  user_id: { type: mongoose.Schema.Types.ObjectId },
  created: { type: Date, default: Date.now }
});

if (!punchcardSchema.options.toJSON) { punchcardSchema.options.toJSON = {}; }
if (!punchcardSchema.options.toObject) { punchcardSchema.options.toObject = {}; }
punchcardSchema.options.toJSON.transform = punchcardSchema.options.toObject.transform = (doc, ret) => {
  delete ret.__v;
};

const Punchcard = mongoose.model('Punchcard', punchcardSchema);

export default Punchcard;
