#!/usr/bin/env python
# _*_ coding: utf-8 _*_
try:  
    from xml.etree.cElementTree import parse, Element
except ImportError:  
    from xml.etree.ElementTree import parse, Element

doc = parse('..\\Project\\Project.uvprojx')
root = doc.getroot()
startAddress = root.find('Targets').find('Target').find('TargetOption').find('TargetArmAds') \
    .find('ArmAdsMisc').find('OnChipMemories').find('OCR_RVCT4').find('StartAddress')
startAddress.text = '0x08000000'

doc.write('..\\Project\\Project.uvprojx', encoding='utf-8', xml_declaration=True)