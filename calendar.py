from PySide.QtCore import *
import string


# Easter 2010 is:
# April 4th

def easter(y):

    c = y/100
    g = y % 19
    k = (c-17)/25
    i = (c - c/4 - (c-k)/3 + 19*g + 15) % 30
    i = i - (i/28)*(1 - (i/28)*(29/(i+1))*((21-g)/11))
    j = (y + y/4 + i + 2 - c + c/4) % 7
    l = i-j
    m = 3+ (l+40)/44
    d = l + 28 - 31*(m/4)

    return QDate(y, m,d)

def adventSunday(y):
    christmas = QDate(y, 12, 25)
    day = christmas.dayOfWeek() % 7
    return christmas.addDays(-day).addDays(-4*7)

class Calendar:

    feasts = {
        0  : { "name": 'Easter', "prec":9 },
        39 : { "name": 'Ascension', "prec":9 },
        49 : { "name": 'Pentecost', "colour": 'red', "prec":9 },
        56 : { "name": 'Trinity', "prec":9 },

        # And others:
            -46 : { "name": 'Ash Wednesday', "colour":'purple', "prec":7 },
        # is the colour of Shrove Tuesday right?
        -47 : { "name": 'Shrove Tuesday', "colour":'white', "prec":7 }, 
        # Actually, Easter Eve doesn't have a colour
        -1 : { "name": 'Easter Eve', "colour":'purple', "prec":7 },
        -2 : { "name": 'Good Friday', "colour":'purple', "prec":7 },

        50 : { "name": 'Book of Common Prayer', "prec":3 },

        # Dates relative to Christmas are encoded as 10000 + 100*m + d
        # for simplicity.

        # Principal Feasts (precedence is 9)

        10106 : {"name":'Epiphany', "prec":9},
        11101 : {"name":'All Saints', "prec":9},
        11225 : {"name":'Christmas', "prec":9},

        # Days which can take priority over Sundays (precedence is 7)

        10101 : {"name":'Holy Name', "prec":7},
        10202 : {"name":'Presentation of our Lord', "prec":7},
        10806 : {"name":'Transfiguration', "prec":7},
        
        # (Precendence of Sundays is 5)
        
        # Days which cannot take priorities over Sundays (precedence is 4
        # if major, 3 otherwise)

        10110 : {"name":'William Laud', "prec":3},
        10113 : {"name":'Hilary', "prec":3},
        10117 : {"name":'Antony', "prec":3},
        10118 : {"name":'Confession of Saint Peter', "prec":4},
        10119 : {"name":'Wulfstan', "prec":3},
        10120 : {"name":'Fabian', "prec":3},
        10121 : {"name":'Agnes', "prec":3},
        10122 : {"name":'Vincent', "martyr":1, "prec":3},
        10123 : {"name":'Phillips Brooks', "prec":3},
        10125 : {"name":'Conversion of Saint Paul', "prec":4},
        10126 : {"name":'Timothy and Titus', "prec":3},
        10127 : {"name":'John Chrysostom', "prec":3},
        10128 : {"name":'Thomas Aquinas', "prec":3},

        10203 : {"name":'Anskar', "prec":3},
        10204 : {"name":'Cornelius', "prec":3},
        10205 : {"name":'Martyrs of Japan', "martyr":1, "prec":3},
        10213 : {"name":'Absalom Jones', "prec":3},
        10214 : {"name":'Cyril and Methodius', "prec":3},
        10215 : {"name":'Thomas Bray', "prec":3},
        10223 : {"name":'Polycarp', "martyr":1, "prec":3},
        10224 : {"name":'Matthias', "prec":4},
        10227 : {"name":'George Herbert', "prec":3},

        10301 : {"name":'David', "prec":3},
        10302 : {"name":'Chad', "prec":3},
        10303 : {"name":'John and Charles Wesley', "prec":3},
        10307 : {"name":'Perpetua and her companions', "martyr":1, "prec":3},
        10308 : {"name":'Gregory of Nyssa', "prec":3},
        10309 : {"name":'Gregory the Great', "prec":3},
        10317 : {"name":'Patrick', "prec":3},
        10318 : {"name":'Cyril', "prec":3},
        10319 : {"name":'Joseph', "prec":4},
        10320 : {"name":'Cuthbert', "prec":3},
        10321 : {"name":'Thomas Ken', "prec":3},
        10322 : {"name":'James De Koven', "prec":3},
        10323 : {"name":'Gregory the Illuminator', "prec":3},
        10325 : {"name":'Annunciation of our Lord', "bvm":1, "prec":4},
        10327 : {"name":'Charles Henry Brent', "prec":3},
        10329 : {"name":'John Keble', "prec":3},
        10331 : {"name":'John Donne', "prec":3},

        10401 : {"name":'Frederick Denison Maurice', "prec":3},
        10402 : {"name":'James Lloyd Breck', "prec":3},
        10403 : {"name":'Richard of Chichester', "prec":3},
        10408 : {"name":'William Augustus Muhlenberg', "prec":3},
        10409 : {"name":'William Law', "prec":3},
        10411 : {"name":'George Augustus Selwyn', "prec":3},
        10419 : {"name":'Alphege', "martyr":1, "prec":3},
        10421 : {"name":'Anselm', "prec":3},
        10425 : {"name":'Mark the Evangelist', "prec":4},
        10429 : {"name":'Catherine of Siena', "prec":3},

        10501 : {"name":'Philip and James', "prec":4},
        10502 : {"name":'Athanasius', "prec":3},
        10504 : {"name":'Monnica', "prec":3},
        10508 : {"name":'Julian of Norwich', "prec":3},
        10509 : {"name":'Gregory of Nazianzus', "prec":3},
        10519 : {"name":'Dustan', "prec":3},
        10520 : {"name":'Alcuin', "prec":3},
        10524 : {"name":'Jackson Kemper', "prec":3},
        10525 : {"name":'Bede', "prec":3},
        10526 : {"name":'Augustine of Canterbury', "prec":3},
        10531 : {"name":'Visitation of Mary', "bvm":1, "prec":4},

        10601 : {"name":'Justin', "prec":3},
        10602 : {"name":'Martyrs of Lyons', "martyr":1, "prec":3},
        10603 : {"name":'Martyrs of Uganda', "martyr":1, "prec":3},
        10605 : {"name":'Boniface', "prec":3},
        10609 : {"name":'Columba', "prec":3},
        10610 : {"name":'Ephrem of Edessa', "prec":3},
        10611 : {"name":'Barnabas', "prec":4},
        10614 : {"name":'Basil the Great', "prec":3},
        10616 : {"name":'Joseph Butler', "prec":3},
        10618 : {"name":'Bernard Mizeki', "prec":3},
        10622 : {"name":'Alban', "martyr":1, "prec":3},
        10624 : {"name":'Nativity of John the Baptist', "prec":3},
        10628 : {"name":'Irenaeus', "prec":3},
        10629 : {"name":'Peter and Paul', "martyr":1, "prec":3},
        10704 : {"name":'Independence Day', "prec":3},
        10711 : {"name":'Benedict of Nursia', "prec":3},
        10717 : {"name":'William White', "prec":3},
        10722 : {"name":'Mary Magdalene', "prec":4},
        10724 : {"name":'Thomas a Kempis', "prec":3},
        10725 : {"name":'James the Apostle', "prec":4},
        10726 : {"name":'Parents of the Blessed Virgin Mary', "bvm":1, "prec":3},
        10727 : {"name":'William Reed Huntington', "prec":3},
        10729 : {"name":'Mary and Martha', "prec":4},
        10730 : {"name":'William Wilberforce', "prec":3},
        10731 : {"name":'Joseph of Arimathaea', "prec":3},
        
        10806 : {"name":'Transfiguration', "prec":4},
        10807 : {"name":'John Mason Neale', "prec":3},
        10808 : {"name":'Dominic', "prec":3},
        10810 : {"name":'Lawrence', "martyr":1, "prec":3},
        10811 : {"name":'Clare', "prec":3},
        10813 : {"name":'Jeremy Taylor', "prec":3},
        10815 : {"name":'Mary the Virgin', "bvm":1, "prec":4},
        10818 : {"name":'William Porcher DuBose', "prec":3},
        10820 : {"name":'Bernard', "prec":3},
        10824 : {"name":'Bartholemew', "prec":4},
        10825 : {"name":'Louis', "prec":3},
        10828 : {"name":'Augustine of Hippo', "prec":3},
        10801 : {"name":'Aidan', "prec":3},

        10902 : {"name":'Martyrs of New Guinea', "martyr":1, "prec":3},
        10912 : {"name":'John Henry Hobart', "prec":3},
        10913 : {"name":'Cyprian', "prec":3},
        10914 : {"name":'Holy Cross', "prec":4},
        10916 : {"name":'Ninian', "prec":3},
        10918 : {"name":'Edward Bouverie Pusey', "prec":3},
        10919 : {"name":'Theodore of Tarsus', "prec":3},
        10920 : {"name":'John Coleridge Patteson and companions', "martyr":1, "prec":3},
        10921 : {"name":'Matthew', "martyr":1, "prec":4},
        10925 : {"name":'Sergius', "prec":3},
        10926 : {"name":'Lancelot Andrewes', "prec":3},
        10929 : {"name":'Michael and All Angels', "prec":4},
        10930 : {"name":'Jerome', "prec":3},

        11001 : {"name":'Remigius', "prec":3},
        11004 : {"name":'Francis of Assisi', "prec":3},
        11006 : {"name":'William Tyndale', "prec":3},
        11009 : {"name":'Robert Grosseteste', "prec":3},
        11015 : {"name":'Samuel Isaac Joseph Schereschewsky', "prec":3},
        11016 : {"name":'Hugh Latimer, Nicholas Ridley, Thomas Cranmer', "martyr":1, "prec":3},
        11017 : {"name":'Ignatius', "martyr":1, "prec":3},
        11018 : {"name":'Luke', "prec":4},
        11019 : {"name":'Henry Martyn', "prec":3},
        11023 : {"name":'James of Jerusalem', "martyr":1, "prec":4},
        11026 : {"name":'Alfred the Great', "prec":3},
        11028 : {"name":'Simon and Jude', "prec":4},
        11029 : {"name":'James Hannington and his companions', "martyr":1, "prec":3},

        11101 : {"name":'All Saints', "prec":4},
        11102 : {"name":'All Faithful Departed', "prec":3},
        11103 : {"name":'Richard Hooker', "prec":3},
        11107 : {"name":'Willibrord', "prec":3},
        11110 : {"name":'Leo the Great', "prec":3},
        11111 : {"name":'Martin of Tours', "prec":3},
        11112 : {"name":'Charles Simeon', "prec":3},
        11114 : {"name":'Consecration of Samuel Seabury', "prec":3},
        11116 : {"name":'Margaret', "prec":3},
        11117 : {"name":'Hugh', "prec":3},
        11118 : {"name":'Hilda', "prec":3},
        11119 : {"name":'Elizabeth of Hungary', "prec":3},
        11123 : {"name":'Clement of Rome', "prec":3},
        11130 : {"name":'Andrew', "prec":4},

        11201 : {"name":'Nicholas Ferrar', "prec":3},
        11202 : {"name":'Channing Moore Williams', "prec":3},
        11204 : {"name":'John of Damascus', "prec":3},
        11205 : {"name":'Clement of Alexandria', "prec":3},
        11206 : {"name":'Nicholas', "prec":3},
        11207 : {"name":'Ambrose', "prec":3},
        11221 : {"name":'Thomas', "prec":4},
        # Christmas is dealt with above
        11226 : {"name":'Stephen', "martyr":1, "prec":4},
        11227 : {"name":'John the Apostle', "prec":4},
        11228 : {"name":'Holy Innocents', "martyr":1, "prec":4},
        }

    def __init__(self, date=None):
        if not date:
            date = QDate.currentDate()

        # Now, what feasts are we celebrating today?

        self.celebrating = self._celebrating(date)

        print [
            self.m_season,
            self.celebrating,
            ]

    def _celebrating(self, date, firstPass=True):
        # "The Church Year consists of two cycles of feasts and holy days: one is
        #  dependent upon the movable date of the Sunday of the Resurrection or
        #  Easter Day; the other, upon the fixed date of December 25, the Feast
        #  of our Lord's Nativity or Christmas Day."
        
        fromChristmas = 10000 + date.month()*100 + date.day()
        fromEaster = easter(date.year()).daysTo(date)

        # Work out the season (but only on the first pass)
        # FIXME; TO HERE; This is broken
        if firstPass:
            if fromEaster>-47 and fromEaster<0:
                self.m_season = 'Lent'
                #$weekno = ($easter_point+50)/7
                # FIXME: The ECUSA calendar seems to indicate that Easter Eve ends
                # Lent *and* begins the Easter season. I'm not sure how. Maybe it's
                # in both? Maybe the daytime is in Lent and the night is in Easter?
            elif fromEaster>=0 and fromEaster<=49:
                # yes, this is correct: Pentecost itself is in Easter season;
                # Pentecost season actually begins on the day after Pentecost.
                # Its proper name is "The Season After Pentecost".
                self.m_season = 'Easter'
                #$weekno = $easter_point/7;
            elif adventSunday(date.year()).daysTo(date) and fromChristmas<=-1:
                self.m_season = 'Advent'
                #$weekno = 1+($christmas_point-$advent_sunday)/7
            elif fromChristmas>=0 and fromChristmas<=11:
                # The Twelve Days of Christmas.
                self.m_season = 'Christmas'
                #$weekno = 1+$christmas_point/7;
            elif fromChristmas>=12 and fromEaster <= -47:
                self.m_season = 'Epiphany'
                #$weekno = 1+($christmas_point-12)/7;
            else:
                self.m_season = 'Pentecost'
                #$weekno = 1+($easter_point-49)/7;

        # So, what are we celebrating today?

        celebrating = []

        for reference in (fromChristmas, fromEaster):
            if self.feasts.has_key(reference):
                celebrating.append(self.feasts[reference])

        if firstPass:
            yesterday = date.addDays(-1)
            def transferred(feast):
                feast['name'] += ' (transferred)'
                feast['prec'] -= 1
                return feast
            celebrating.extend([transferred(x)
                                for x in self._celebrating(yesterday,
                                                           False)][1:])

        # Maybe it's a Sunday.
        # (But only check if firstPass, because
        # Sundays don't transfer!)

        if firstPass and date.dayOfWeek()==7:
            celebrating.append({'name': '%s (weekno)' % (self.m_season,
                                                         ),
                                'prec': 5})

        return celebrating
        

def self_test():
    for test in file('tests/2006.txt').readlines():
        fields = string.split(test[:-1], None, 3)
        print fields
        (y, m, d) = fields[0].split('-')
        date = QDate(int(y), int(m), int(d))
        calendar = Calendar(date)

if __name__=="__main__":
    print 'Running self-tests.'
    self_test()

