#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include "hw.h"
#include "ble.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

/* Callbacks de conexión Bluetooth */
static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Conexión fallida (error %d)", err);
        return;
    }

    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Dispositivo conectado: %s", addr);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Dispositivo desconectado: %s (razón: 0x%02x)", addr, reason);
}

static struct bt_conn_cb conn_callbacks = {
    .connected = on_connected,    // Se ejecuta al conectar
    .disconnected = on_disconnected,  // Opcional: para detectar desconexiones
};

/* Custom 128-bit UUIDs */
#define VIBRATOR_SERVICE_UUID_VAL \
    BT_UUID_128_ENCODE(0x771a69d3, 0xc9fc, 0x4674, 0x9bce, 0xca5c48f5ba55)

#define VIBRATOR_COMMAND_UUID_VAL \
    BT_UUID_128_ENCODE(0x73d978f7, 0x7dda, 0x4ef2, 0x8cb8, 0xc5e7ae10cc2a)

static struct bt_uuid_128 vibrator_service_uuid = BT_UUID_INIT_128(VIBRATOR_SERVICE_UUID_VAL);
static struct bt_uuid_128 vibrator_command_uuid = BT_UUID_INIT_128(VIBRATOR_COMMAND_UUID_VAL);

/* Command buffer */
#define MAX_CMD_LEN 64
static char vibrator_cmd[MAX_CMD_LEN + 1];

static ssize_t write_vibrator_cmd(struct bt_conn *conn,
                                  const struct bt_gatt_attr *attr,
                                  const void *buf, uint16_t len,
                                  uint16_t offset, uint8_t flags)
{
    if (offset != 0 || len == 0 || len > MAX_CMD_LEN) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memset(vibrator_cmd, 0, sizeof(vibrator_cmd));
    memcpy(vibrator_cmd, buf, len);
    vibrator_control(vibrator_cmd);

    return len;
}

BT_GATT_SERVICE_DEFINE(vibrator_service,
    BT_GATT_PRIMARY_SERVICE(&vibrator_service_uuid),
    BT_GATT_CHARACTERISTIC(&vibrator_command_uuid.uuid,
                           BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE,
                           NULL, write_vibrator_cmd, NULL),
);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, VIBRATOR_SERVICE_UUID_VAL),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

static void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed: %d", err);
        return;
    }

    bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    LOG_INF("Advertising started");
}

void vibrator_ble_init(void)
{
    // callback de conexión
    bt_conn_cb_register(&conn_callbacks);

    // Inicializar Bluetooth
    int err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("Bluetooth enable failed: %d", err);
    }
}

