from collections import defaultdict
from skidl import Pin, Part, Alias, SchLib, SKIDL, TEMPLATE

from skidl.pin import pin_types

SKIDL_lib_version = '0.0.1'

apachon_8knobs = SchLib(tool=SKIDL).add_parts(*[
        Part(**{ 'name':'Conn_02x12_Odd_Even', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Conn_02x12_Odd_Even'}), 'ref_prefix':'J', 'fplist':[''], 'footprint':'Connector_PinHeader_2.54mm:PinHeader_2x12_P2.54mm_Vertical', 'keywords':'connector', 'description':'Generic connector, double row, 02x12, odd/even pin numbering scheme (row 1 odd numbers, row 2 even numbers), script generated (kicad-library-utils/schlib/autogen/connector/)', 'datasheet':'', 'pins':[
            Pin(num='1',name='Pin_1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='Pin_2',func=pin_types.PASSIVE,unit=1),
            Pin(num='3',name='Pin_3',func=pin_types.PASSIVE,unit=1),
            Pin(num='4',name='Pin_4',func=pin_types.PASSIVE,unit=1),
            Pin(num='5',name='Pin_5',func=pin_types.PASSIVE,unit=1),
            Pin(num='6',name='Pin_6',func=pin_types.PASSIVE,unit=1),
            Pin(num='7',name='Pin_7',func=pin_types.PASSIVE,unit=1),
            Pin(num='8',name='Pin_8',func=pin_types.PASSIVE,unit=1),
            Pin(num='9',name='Pin_9',func=pin_types.PASSIVE,unit=1),
            Pin(num='10',name='Pin_10',func=pin_types.PASSIVE,unit=1),
            Pin(num='11',name='Pin_11',func=pin_types.PASSIVE,unit=1),
            Pin(num='12',name='Pin_12',func=pin_types.PASSIVE,unit=1),
            Pin(num='13',name='Pin_13',func=pin_types.PASSIVE,unit=1),
            Pin(num='14',name='Pin_14',func=pin_types.PASSIVE,unit=1),
            Pin(num='15',name='Pin_15',func=pin_types.PASSIVE,unit=1),
            Pin(num='16',name='Pin_16',func=pin_types.PASSIVE,unit=1),
            Pin(num='17',name='Pin_17',func=pin_types.PASSIVE,unit=1),
            Pin(num='18',name='Pin_18',func=pin_types.PASSIVE,unit=1),
            Pin(num='19',name='Pin_19',func=pin_types.PASSIVE,unit=1),
            Pin(num='20',name='Pin_20',func=pin_types.PASSIVE,unit=1),
            Pin(num='21',name='Pin_21',func=pin_types.PASSIVE,unit=1),
            Pin(num='22',name='Pin_22',func=pin_types.PASSIVE,unit=1),
            Pin(num='23',name='Pin_23',func=pin_types.PASSIVE,unit=1),
            Pin(num='24',name='Pin_24',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'CD74HC4067M', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'CD74HC4067M'}), 'ref_prefix':'U', 'fplist':['Package_SO:SOIC-24W_7.5x15.4mm_P1.27mm'], 'footprint':'Package_SO:SOIC-24W_7.5x15.4mm_P1.27mm', 'keywords':'multiplexer demultiplexer mux demux', 'description':'High-Speed CMOS Logic 16-Channel Analog Multiplexer/Demultiplexer, SOIC-24', 'datasheet':'http://www.ti.com/lit/ds/symlink/cd74hc4067.pdf', 'pins':[
            Pin(num='1',name='COM',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='I7',func=pin_types.PASSIVE,unit=1),
            Pin(num='3',name='I6',func=pin_types.PASSIVE,unit=1),
            Pin(num='4',name='I5',func=pin_types.PASSIVE,unit=1),
            Pin(num='5',name='I4',func=pin_types.PASSIVE,unit=1),
            Pin(num='6',name='I3',func=pin_types.PASSIVE,unit=1),
            Pin(num='7',name='I2',func=pin_types.PASSIVE,unit=1),
            Pin(num='8',name='I1',func=pin_types.PASSIVE,unit=1),
            Pin(num='9',name='I0',func=pin_types.PASSIVE,unit=1),
            Pin(num='10',name='S0',func=pin_types.INPUT,unit=1),
            Pin(num='11',name='S1',func=pin_types.INPUT,unit=1),
            Pin(num='12',name='GND',func=pin_types.PWRIN,unit=1),
            Pin(num='13',name='S3',func=pin_types.INPUT,unit=1),
            Pin(num='14',name='S2',func=pin_types.INPUT,unit=1),
            Pin(num='15',name='~{E}',func=pin_types.INPUT,unit=1),
            Pin(num='16',name='I15',func=pin_types.PASSIVE,unit=1),
            Pin(num='17',name='I14',func=pin_types.PASSIVE,unit=1),
            Pin(num='18',name='I13',func=pin_types.PASSIVE,unit=1),
            Pin(num='19',name='I12',func=pin_types.PASSIVE,unit=1),
            Pin(num='20',name='I11',func=pin_types.PASSIVE,unit=1),
            Pin(num='21',name='I10',func=pin_types.PASSIVE,unit=1),
            Pin(num='22',name='I9',func=pin_types.PASSIVE,unit=1),
            Pin(num='23',name='I8',func=pin_types.PASSIVE,unit=1),
            Pin(num='24',name='VCC',func=pin_types.PWRIN,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'C', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C'}), 'ref_prefix':'C', 'fplist':[''], 'footprint':'Capacitor_SMD:C_0805_2012Metric', 'keywords':'cap capacitor', 'description':'Unpolarized capacitor', 'datasheet':'', 'pins':[
            Pin(num='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'R_Potentiometer', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R_Potentiometer'}), 'ref_prefix':'RV', 'fplist':[''], 'footprint':'Potentiometer_THT:Potentiometer_Alps_RK163_Single_Horizontal', 'keywords':'resistor variable', 'description':'Potentiometer', 'datasheet':'', 'pins':[
            Pin(num='1',name='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='2',func=pin_types.PASSIVE,unit=1),
            Pin(num='3',name='3',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] })])