import mongoose from 'mongoose';
import bcrypt from 'bcrypt';

const TEMP_TOKEN_PREFIX = 'temp-';

function genToken() {
  return TEMP_TOKEN_PREFIX + (new mongoose.Types.ObjectId).toHexString();
}

const GENDERS = ['m', 'f', 'o'];
const userSchema = new mongoose.Schema({
  username: { type: String, trim: true, unique: true, minlength: 1 },
  email: { type: String, trim: true, unique: true, minlength: 1 },
  password: { type: String, minlength: 6, set: (password) => bcrypt.hashSync(password, bcrypt.genSaltSync()) },
  token: { type: String, trim: true, unique: true, default: genToken },
  age: { type: Number, min: 0, max: 200 },
  gender: { type: String, enum: GENDERS }
});

if (!userSchema.options.toJSON) { userSchema.options.toJSON = {}; }
if (!userSchema.options.toObject) { userSchema.options.toObject = {}; }
userSchema.options.toJSON.transform = userSchema.options.toObject.transform = (doc, ret) => {
  delete ret.password;
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

userSchema.statics.generateToken = function () {
  return (new mongoose.Types.ObjectId).toHexString();
}

userSchema.methods.verifyPassword = function (password) {
  return bcrypt.compareSync(password, this.password);
};

userSchema.methods.isTokenValid = function () {
  return !this.token.startsWith(TEMP_TOKEN_PREFIX);
};

const User = mongoose.model('User', userSchema);

export default User;
