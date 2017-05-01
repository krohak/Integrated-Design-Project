/* Magic Mirror Config Sample
*
* By Michael Teeuw http://michaelteeuw.nl
* MIT Licensed.
*/
var config = {
	"port": 8080,
	"language": "en",
	"timeFormat": 24,
	"units": "metric",
	"modules": [
		{
			"module": "MMM-Admin-Interface"
		},
		{
                        module: 'newsfeed',
                        position: 'bottom_bar',
                        config: {
                                feeds: [
                                        {
                                                title: "New York Times",
                                                url: "http://www.nytimes.com/services/xml/rss/nyt/HomePage.xml"
                                        }
                                ],
                                showSourceTitle: true,
                                showPublishDate: true
                		}
		},
		{
                                     "module": "googlemap",
                                     "position": "top_left",
                                     "config": {
                                             "apikey": "AIzaSyB87PjoPJY8-itJtBcVDQsCmRFPU9O6TrE",
                                             "origin": "kennedy town",
                                             "destination": "hku"
                                     }
                },
		{
                                     "module": "MMM-HK-Transport",
                                     "position": "bottom_right",
                                     "config": {
                                             "stopID": "HKStop_KowloonCentralPostOffice_N_3_1",
                                             "stopName": "九龍中央郵政局"
                                     }
                },
		{
			"module": "MMM-SimpleLogo",
        		"position": "top_right",    // This can be any of the regions.
        		"config": {
					"fileUrl":"modules/MMM-SimpleLogo/public/IMAGE.JPG",
					"width":"200px",
					"refreshInterval":"1000"
            		         }
		},
		{
		module: 'MMM-Rest',
		position: 'bottom_left',	// This can be any of the regions.
									// Best results in one of the side regions like: top_left
        config: {
                debug: false,
                mappings: {
                    on_off: {
                        true: 'on',
                        false: 'off',
                    },
                    temperature: {
                        1: 'cold',
                        2: 'warm',
                        3: 'HOT',
                    },
                },
                sections: [
                {
                    format: '%.2f<span class="wi wi-celsius"></span>',
                    url: 'http://localhost/mirror/celsius',
                },
                {
                    format: '%d%%<span class="wi wi-humidity"></span>',
                    url: 'http://localhost/mirror/humidity',
                },
                {
                    format: '%s',
                    mapping: 'temperature',
                    url: 'https://www.dirk-melchers.de/echo.php?text=2',
                },
                {
                    format: '%d<span class="wi wi-humidity"></span>',
                    url: 'https://www.dirk-melchers.de/echo.php?text=62.1',
                },
                {
                    format: 'Lights %s',
                    mapping: 'on_off',
                    url: 'https://www.dirk-melchers.de/echo.php?text=true',
                },
		{
                    format: '%.2f<span class="wi wi-celsius"></span>',
                    url: 'http://localhost/mirror/target',
                },
		{
                    format: '%d',
                    url: 'http://localhost/mirror/bright',
                },
		{
                    format: '%d',
                    url: 'http://localhost/mirror/max',
                },

            ],
            output: [
                ['Temperature','@1'],
		['Humidity','@2'],
		['TargetTemperature','@6'],
		['CurrentBrightness','@7'],
		['MaxBrightness','@8'],
                ['Kitchen','@3','@4'],
                ['Fridge','@5'],
            ],
		updateInterval:"5000"
	    },
	}
	],
	"ipWhitelist": [
		"127.0.0.1",
		"::ffff:127.0.0.1",
		"::1",
		"192.168.1.2",
		"147.8.98.61",
		"192.168.X.1/24",
		"::ffff:192.168.X.1/24",
		"192.168.1.27"
	]
};
/*************** DO NOT EDIT THE LINE BELOW ***************/
if (typeof module !== 'undefined') {module.exports = config;}
