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

int medianAnalogRead(byte pin)
{
#ifdef _DEBUG
	unsigned long t = micros();
#endif

	struct Bucket 
	{
		int Value;
		byte Count;
	};

	static const byte SampleCount = 32;
	static const byte AveragedBucketCount = 3;
	static Bucket buckets[16];

	memset(&buckets, 0, sizeof buckets);

	byte bucketCount = 0;
	for (byte i = 0; i < SampleCount; i++)
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
			// skip "most hit" test/set for count == 1
		}
	}

	byte mostHitBuckets[AveragedBucketCount];
	byte mostHits = 0;

	memset(&mostHitBuckets, 0, sizeof mostHitBuckets);

	for (byte i = 0; i < bucketCount; i++)
		if (mostHits < buckets[i].Count)
		{
			for (byte j = 1; j < AveragedBucketCount; j++)
				mostHitBuckets[j] = mostHitBuckets[j - 1];
			mostHitBuckets[0] = i;
			mostHits = buckets[i].Count;
		}

	int weight = 0;
	long accum = 0;
	for (byte i = 0; i < AveragedBucketCount; i++)
	{
		const Bucket& bucket = buckets[mostHitBuckets[i]];
		accum += bucket.Value * bucket.Count;
		weight += bucket.Count;
	}

	int value = (int) round(accum / (float) weight);

#ifdef _DEBUG
	t = micros() - t;
#endif
	trace(P("sampled %i from %hhu buckets (%i, %i, %i) in %u ms"), value, bucketCount, buckets[mostHitBuckets[0]].Value, buckets[mostHitBuckets[1]].Value, buckets[mostHitBuckets[2]].Value, (unsigned int) (t / 1000));

	return value;
}
