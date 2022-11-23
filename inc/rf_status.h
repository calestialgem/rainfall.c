/* Provides a way of communicating the result of a function call. */
#ifndef RF_STATUS_H
#define RF_STATUS_H 1

/* Information on the result of an operation. */
enum rf_status {
  /* Operation was completed without any errors. */
  RF_SUCCEEDED,
  /* Operation did not start because some requirements were not met. */
  RF_CANCELED,
  /* Operation started, but it was aborted because of an error. */
  RF_FAILED,
};

#endif // RF_STATUS_H
