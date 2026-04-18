# GraduationProject

## 椤圭洰姒傝堪

鏈粨搴撶敤浜庢瘯涓氳璁″紑鍙戯紝褰撳墠涓荤嚎宸茬粡杩涘叆绋冲畾鑱旇皟闃舵锛屾牳蹇冮棴鐜负锛?

`灏忔櫤璇煶/璋冭瘯鐘舵€?-> ESP32 鍔ㄤ綔鏄犲皠 -> 鏈湴 I2C 鑷畾涔夊崗璁?-> AI8051U 鎺ユ敹鎵ц -> WS2812 LED 鐭╅樀鏄剧ず`

褰撳墠鐗堟湰宸茬粡瀹屾垚涓ゆ潯涓荤嚎鐨勭ǔ瀹氭敹鍙ｏ細

1. `STC51/Project/ws2812_driver/`
   - AI8051U 渚?WS2812 澶嶇敤鎵弿鏄剧ず绯荤粺銆?
   - 宸插叿澶囩ǔ瀹氱殑 PWM + DMA銆?4HC595 琛岄€夈€丳MOS 楂樹晶鍒囨崲銆乁SB 璋冭瘯涓?I2C 浠庢満閾捐矾銆?
2. `External/xiaozhi-esp32/`
   - 灏忔櫤 ESP32 鍙傝€冨揩鐓у強鏈」鐩墿灞曘€?
   - 宸插叿澶囪闊抽鑹?棰勮鏄犲皠銆佺煩闃靛姩浣滃璞′笅鍙戙€丄CK 璇诲洖銆佺ǔ瀹氱増璋冭瘯鐣岄潰涓庢湰鍦?HTTP 鎴浘閾捐矾銆?

## 褰撳墠绋冲畾鐗堣兘鍔?

### STC51 / AI8051U / WS2812 渚?

- PWM + DMA 鍙岄€氶亾杈撳嚭閾捐矾绋冲畾杩愯銆?
- 74HC595 + PMOS 琛屾壂鎻忋€佸浣嶅熬娉㈠拰瀹氭椂鑺傛媿绛栫暐宸叉敹鍙ｃ€?
- 宸叉敮鎸?`normal_pair` 涓?`legacy_shift` 涓ょ被鎵弿/鍙戦€佹ā寮忋€?
- 宸叉敮鎸?USB 璋冭瘯鍛戒护锛氶鑹层€佸浘妗堛€侀棿闅斻€佹覆鏌撴ā寮忓垏鎹€?
- 宸叉敮鎸?AI8051U I2C 浠庢満銆佸崗璁В鏋愩€佸姩浣滃垎鍙戙€佺姸鎬?閿欒鍥炲寘銆?

### XiaoZhi / ESP32 / GP_Port 渚?

- 宸叉帴鍏ュ叡浜崗璁ご `gp_led_matrix_protocol.h`銆?
- 宸叉帴閫?ESP32 渚х煩闃甸┍鍔?`gp_led_matrix_esp32.h/.cc`銆?
- 宸叉帴閫?AI8051U 鎺ュ彛灞?`gp_led_matrix_ai8051u.h/.c` 涓庡姩浣滄墽琛屽眰 `gp_led_action.c`銆?
- 宸叉敮鎸佽闊抽鑹茬粨鏋滀笌璋冭瘯鍦嗙偣鐘舵€佸悓姝ュ埌 LED 鐭╅樀銆?
- 宸叉敮鎸佺煩闃甸璁撅細`diamond`銆乣cross`銆乣JLU_emblem`銆乣scroll_subtitle`銆?
- 宸叉敮鎸佲€滄湭鎸囧畾棰勮鏃堕粯璁ょ函鑹叉弧灞忊€濆拰鈥滀粎鍦ㄦ樉寮忓浘鍍忔洿鏂版椂閫氫俊鈥濈瓥鐣ャ€?
- 宸叉敮鎸佸浘妗堣儗鏅壊鐙珛鎺у埗锛屾寚瀹氶璁惧悗涓嶅啀鑷姩杞挱鍏朵粬娴嬭瘯鍥炬銆?
- 宸叉敮鎸佺ǔ瀹氱増璋冭瘯鐣岄潰锛歚DBG` 鍏ュ彛銆佸浐瀹氭爣棰樻爮 `Back / Debug Menu / S`銆佸崟椤佃Е鎽告帶鍒跺尯銆佸渾鐐归瑙堝尯銆侀摼璺姸鎬佸尯鍜屾憳瑕佷俊鎭尯銆?
- 宸叉敮鎸佹湰鍦?HTTP 鎴浘锛氳澶囦晶 `S` 鎸夐敭鐩翠紶 `/snapshot`锛屼富鏈轰晶閫氳繃 `/control/snapshot` 瑙﹀彂璁惧鎴浘銆?

## 浠撳簱缁撴瀯

```text
GraduationProject/
|-- README.md
|-- Doc/
|   `-- 椤圭洰鏂囨。/
|       |-- usb_play_v2_guide.md
|       |-- ws2812_driver_current_implementation.md
|       |-- xiaozhi_ai8051u_i2c_interface_protocol.md
|       `-- xiaozhi_esp32_porting_summary.md
|-- External/
|   `-- xiaozhi-esp32/
|       |-- main/                      # 灏忔櫤搴旂敤銆佹澘绾у拰璁惧鎶借薄
|       `-- GP_Port/                   # 鏈」鐩墿灞曞崗璁€侀┍鍔ㄣ€佽仈璋冭剼鏈拰璇存槑
|-- STC51/
|   `-- Project/
|       `-- ws2812_driver/
|           |-- Sources/
|           |   |-- app/               # 鎵弿璋冨害涓庡簲鐢ㄥ眰娴佺▼
|           |   |-- mid/               # 娓叉煋銆佸姩鐢汇€佹寜閿帶鍒?
|           |   |-- drv/               # WS2812 / 74HC595 椹卞姩
|           |   |-- inc/               # 鍏变韩澶存枃浠朵笌閰嶇疆
|           |   |-- timer.c            # 瀹氭椂鍣ㄤ笌鑺傛媿鎺у埗
|           |   |-- usblib.c           # USB 鍛戒护鍏ュ彛
|           |   `-- main.c             # MCU 鍏ュ彛涓庡垵濮嬪寲
|           `-- ws2812_driver.uvproj
`-- .github/
    `-- prompts/                       # 椤圭洰寮€鍙?prompt 闆嗗悎
```

## 鍏抽敭鏂囨。鍏ュ彛

- 灏忔櫤绉绘涓庤仈璋冩€荤粨锛歚Doc/椤圭洰鏂囨。/xiaozhi_esp32_porting_summary.md`
- 灏忔櫤涓?AI8051U I2C 鍗忚璇存槑锛歚Doc/椤圭洰鏂囨。/xiaozhi_ai8051u_i2c_interface_protocol.md`
- WS2812 椹卞姩瀹炵幇璇存槑锛歚Doc/椤圭洰鏂囨。/ws2812_driver_current_implementation.md`
- GP_Port 鎬昏锛歚External/xiaozhi-esp32/GP_Port/gp_port_project_overview.md`
- 璋冭瘯鐣岄潰涓庢埅鍥句娇鐢ㄨ鏄庯細`External/xiaozhi-esp32/GP_Port/gp_debug_feature_usage.md`
- MCP 宸ュ叿璇存槑锛歚External/xiaozhi-esp32/GP_Port/gp_mcp_tools.md`

## 鏋勫缓涓庨獙璇?

### STC51 宸ョ▼

1. 浣跨敤 Keil 鎵撳紑 `STC51/Project/ws2812_driver/ws2812_driver.uvproj`銆?
2. 缂栬瘧骞跺湪闇€瑕佹椂閫氳繃 STC ISP 涓嬭浇鍥轰欢銆?
3. 閫氳繃涓插彛鎴?USB 鍛戒护楠岃瘉棰滆壊銆佸浘妗堛€侀棿闅斿拰娓叉煋妯″紡鍒囨崲銆?

### ESP32 宸ョ▼

1. 浣跨敤 ESP-IDF 鎻掍欢鎵撳紑 `External/xiaozhi-esp32/`銆?
2. 閫夋嫨 `lichuang-dev` 骞舵墽琛屾瀯寤恒€?
3. 杩愯 `GP_Port/gp_mcp_endpoint_client.py`锛岃仈璋?`/snapshot`銆乣/control/snapshot` 鍜?MCP 宸ュ叿閾俱€?

## 鎻愪氦杈圭晫

榛樿涓嶆彁浜や互涓嬬函鏈湴浜х墿锛?

- `*.uvgui.*`
- `*.uvopt`
- `__pycache__/`
- `*.pyc`
- 涓存椂瀵煎嚭鎴浘銆佹祴璇曞浘鐗囧拰涓存椂鏃ュ織
