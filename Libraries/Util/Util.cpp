#include "Util.h"
#include "Arduino.h"

bool boolAnalogRead(byte pin)
{
	return analogRead(pin) > ANALOG_HIGH;
}

float floatAnalogRead(byte pin)
{
	return analogRead(pin) / 1023.0f;
}

void floatAnalogWrite(byte pin, float value)
{
	analogWrite(pin, (int) round(value * 255));
}

int smartMedianAnalogRead(byte pin)
{
//#ifdef _DEBUG
//	unsigned long t = micros();
//#endif

	struct Bucket 
	{
		int Value;
		byte Count;
	};

	static Bucket buckets[MedianMaxBuckets];

	memset(&buckets, 0, sizeof buckets);

	byte bucketCount = 0;
	for (byte i = 0; i < MedianSampleCount; i++)
	{
		int sample = analogRead(pin);

		bool bucketFound = false;
		for (byte j = 0; j < bucketCount; j++)
			if (buckets[j].Value == sample)
			{
				bucketFound = true;
				buckets[j].Count++;
				break;
			}

		if (!bucketFound)
		{
			bucketCount++;
			buckets[bucketCount].Value = sample;
			buckets[bucketCount].Count = 1;
		}
	}

	Bucket* averagedBuckets[MedianAveragedBuckets];
	for (byte i = 0; i < MedianAveragedBuckets; i++)
		averagedBuckets[i] = &buckets[i];

	for (byte i = MedianAveragedBuckets; i < bucketCount; i++)
	{
		byte thisCount = buckets[i].Count;
		for (byte j = 0; j < MedianAveragedBuckets; j++)
			if (thisCount >= averagedBuckets[j]->Count)
			{
				averagedBuckets[j] = &buckets[i];
				break;
			}
	}
		
	int weight = 0;
	long accum = 0;
	int minValue = 32767, maxValue = 0;
	for (byte i = 0; i < MedianAveragedBuckets; i++)
	{
		const Bucket& bucket = *averagedBuckets[i];

		minValue = min(minValue, bucket.Value);
		maxValue = max(maxValue, bucket.Value);

		accum += bucket.Value * bucket.Count;
		weight += bucket.Count;
	}

	int value;
	if (maxValue - minValue > MedianMaxVariance)
		value = 0;
	else
		value = (int) floor(accum / (float) weight);

//#ifdef _DEBUG
//	t = micros() - t;
//#endif
//	trace(P("%i < %hhu (%i-%hhu, %i-%hhu, %i-%hhu, %i-%hhu, %i-%hhu) [%i ~ %i] (%ums)"), 
//		value, bucketCount, 
//		averagedBuckets[0] == NULL ? 0 : averagedBuckets[0]->Value, averagedBuckets[0] == NULL ? 0 : averagedBuckets[0]->Count,
//		averagedBuckets[1] == NULL ? 0 : averagedBuckets[1]->Value, averagedBuckets[1] == NULL ? 0 : averagedBuckets[1]->Count,
//		averagedBuckets[2] == NULL ? 0 : averagedBuckets[2]->Value, averagedBuckets[2] == NULL ? 0 : averagedBuckets[2]->Count,
//		averagedBuckets[3] == NULL ? 0 : averagedBuckets[3]->Value, averagedBuckets[3] == NULL ? 0 : averagedBuckets[3]->Count,
//		averagedBuckets[4] == NULL ? 0 : averagedBuckets[4]->Value, averagedBuckets[4] == NULL ? 0 : averagedBuckets[4]->Count,
//		minValue, maxValue,
//		(unsigned int) (t / 1000.0f));

	return value;
}
