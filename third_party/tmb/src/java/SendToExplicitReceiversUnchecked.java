//   Copyright 2014-2015 Quickstep Technologies LLC.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

package tmb.voltdb;

import org.voltdb.*;

public class SendToExplicitReceiversUnchecked extends VoltProcedure {
  public final SQLStmt insertQueuedMessage = new SQLStmt(
      "INSERT INTO queued_message VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);");
  public final SQLStmt insertCancellableMessage = new SQLStmt(
      "INSERT INTO cancellable_message VALUES(?);");

  private static final org.voltdb.types.TimestampType MAX_TIMESTAMP
      = new org.voltdb.types.TimestampType(Long.MAX_VALUE);

  public long run(int sender_id,
                  int[] receiver_ids,
                  org.voltdb.types.TimestampType send_time,
                  org.voltdb.types.TimestampType expiration_time,
                  short priority,
                  int type_id,
                  byte[] payload,
                  byte cancellable) throws VoltAbortException {
    org.voltdb.types.TimestampType actual_expiration_time
        = (expiration_time == null
           || expiration_time.getTime() == VoltType.NULL_BIGINT)
               ? MAX_TIMESTAMP : expiration_time;

    for (int receiver_id : receiver_ids) {
      voltQueueSQL(insertQueuedMessage,
                   receiver_id,
                   getUniqueId(),
                   sender_id,
                   send_time,
                   actual_expiration_time,
                   priority,
                   type_id,
                   payload,
                   payload.length);
    }

    if (cancellable != 0) {
      voltQueueSQL(insertCancellableMessage, getUniqueId());
    }

    voltExecuteSQL();

    return getUniqueId();
  }
}
