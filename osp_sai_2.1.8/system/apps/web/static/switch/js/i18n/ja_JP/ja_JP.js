;Public.app.config(['$translateProvider', function($translateProvider) {
    $translateProvider.translations('ja_JP', {
        /* usual */
        'Hi': 'こんにちわ',
        'Menu': 'メニュー',
        'Login': 'ログイン',
        'Logout': 'ログアウト',
        'Username': 'ユーザ',
        'Password': 'パスワード',
        'Options': 'オプション',
        'Count': '繰り返す',

        /* navigation bar */
        'Device Summary': 'デバイス概要',
        'System Management': 'システムマネジャー',
        'File Management': 'ファールマネジャー',
        'System Configuration': 'システム設定',
        'Load Configuration': '設定のロード',
        'Log Management': 'ログマネジャー',
        'SNMP Configuration': 'SNMP 設定',
        'SNMP Trap Configuration': 'SNMP Trap 設定',
        
        'Interface Management': 'インターフェース',
        'Ethernet Status': 'イーサネット状態',
        'Ethernet Stats': 'イーサネット統計',
        'Link Aggregation': 'リンク凝集',

        /* tools */
        'Tools': 'ツール',
        'Console': 'コンソール',
        'Destination IP': '伝送先IP',
    });
}])
