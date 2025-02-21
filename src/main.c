#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

// Custom service UUID
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static struct bt_uuid_128 custom_service_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

// Characteristic UUID
#define BT_UUID_CUSTOM_CHAR_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x4321, 0x8765, 0x4321, 0x56789abcdef0)

static struct bt_uuid_128 custom_char_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_CHAR_VAL);

static uint8_t response_data[] = {'O', 'K', '!', '\n'};

static ssize_t ble_write_cb(struct bt_conn *conn,
			    const struct bt_gatt_attr *attr,
			    const void *buf,
			    uint16_t len,
			    uint16_t offset,
			    uint8_t flags)
{
	LOG_INF("Received data: %s", (const char *)buf);
	
	// Send response
	bt_gatt_notify(conn, attr, response_data, sizeof(response_data));
	return len;
}

BT_GATT_SERVICE_DEFINE(custom_service,
	BT_GATT_PRIMARY_SERVICE(&custom_service_uuid),
	BT_GATT_CHARACTERISTIC(&custom_char_uuid.uuid,
			       BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_WRITE,
			       NULL, ble_write_cb, NULL),
	BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static struct bt_conn *current_conn;

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}

	current_conn = conn;
	LOG_INF("Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason %u)", reason);
	current_conn = NULL;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	LOG_INF("Bluetooth initialized");
	
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, NULL, 0, NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}
	
	LOG_INF("Advertising successfully started");
}

int main(void)
{
	int err;

	err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}

	while (1) {
		k_sleep(K_SECONDS(1));
	}
}
