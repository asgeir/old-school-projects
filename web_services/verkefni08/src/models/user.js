import mongoose from 'mongoose';

function genToken() {
  return (new mongoose.Types.ObjectId).toHexString();
}

const GENDERS = ['m', 'f', 'o'];
const userSchema = new mongoose.Schema({
  name: { type: String, trim: true },
  token: { type: String, trim: true, unique: true, default: genToken },
  age: { type: Number, min: 0, max: 200 },
  gender: { type: String, enum: GENDERS }
});

if (!userSchema.options.toJSON) { userSchema.options.toJSON = {}; }
if (!userSchema.options.toObject) { userSchema.options.toObject = {}; }
userSchema.options.toJSON.transform = userSchema.options.toObject.transform = (doc, ret) => {
  delete ret.token;
  delete ret.__v;
};

userSchema.statics.findByToken = function (token, callback) {
  return this.findOne({ token }, (err, user) => {
    if (err) {
      return callback(err);
    }

    if (!user) {
      return callback(new Error('Unable to find user'));
    }

    return callback(false, user);
  });
};

const User = mongoose.model('User', userSchema);

export default User;
