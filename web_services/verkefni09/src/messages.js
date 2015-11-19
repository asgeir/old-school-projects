import kafka from 'kafka-node';

let connectionString = null;
export function getClient(clientId) {
  return new kafka.Client(connectionString, clientId);
}

export function initMessages(connection_string) {
  connectionString = connection_string;
}
