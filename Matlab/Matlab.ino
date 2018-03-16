
readChannelID = CHANNEL-ID; // TODO - replace CHANNEL-ID with your ThingSpeak channel ID
LightFieldID = 2;
readAPIKey = 'CHANNEL-READAPIKEY'; // TODO - replace CHANNEL-READAPIKEY with your ThingSpeak channel read API key

lightData = thingSpeakRead(readChannelID, 'Fields', LightFieldID, 'NumPoints', 60, 'ReadKey', readAPIKey);
A = [lightData];
M = movmean(A,10)

x = linspace(0,60,60); // Generate linearly spaced vector for x axis

thingSpeakScatter(x, lightData, 'xlabel', 'Time, s', 'ylabel', 'Average Light Intensity, V'); // Graph the running average
display(lightData); // Print the running average
