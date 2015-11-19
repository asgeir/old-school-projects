import {HighLevelConsumer} from 'kafka-node';
import {getClient as kafkaGetClient} from '../messages';
import User from '../models/user';

let consumer = null;
function onMessage(message) {
  const userMessage = JSON.parse(message.value);

  User.findOne({ username: userMessage.username }, (err, user) => {
    if (err) {
      console.log('token_daemon === Error looking up user: ' + err);
      return;
    }

    if (!user) {
      console.log('token_daemon === User not found');
      return;
    }

    user.token = User.generateToken();
    user.save((err) => {
      if (err) {
        console.log('token_daemon === Error setting token for user: ' + err);
      }
    });
  });
}

export function initialize(config) {
  consumer = new HighLevelConsumer(kafkaGetClient('token-daemon'), [{ topic: 'users' }], { groupId: 'token-daemon-group' });
  consumer.on('message', onMessage);
}
